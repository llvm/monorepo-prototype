//===---- Query.cpp - -----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "mlir/Query/Query.h"
#include "QueryParser.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/IRMapping.h"
#include "mlir/Query/Matcher/MatchFinder.h"
#include "mlir/Query/QuerySession.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <unordered_map>
#include <unordered_set>

namespace mlir::query {

QueryRef parse(llvm::StringRef line, const QuerySession &qs) {
  return QueryParser::parse(line, qs);
}

std::vector<llvm::LineEditor::Completion>
complete(llvm::StringRef line, size_t pos, const QuerySession &qs) {
  return QueryParser::complete(line, pos, qs);
}

static void printMatch(llvm::raw_ostream &os, QuerySession &qs, Operation *op,
                       const std::string &binding) {
  auto fileLoc = op->getLoc()->findInstanceOf<FileLineColLoc>();
  auto smloc = qs.getSourceManager().FindLocForLineAndColumn(
      qs.getBufferId(), fileLoc.getLine(), fileLoc.getColumn());
  qs.getSourceManager().PrintMessage(os, smloc, llvm::SourceMgr::DK_Note,
                                     "\"" + binding + "\" binds here");
}

// TODO: Extract into a helper function that can be reused outside query
// context.
static Operation *extractFunction(std::vector<Operation *> &ops,
                                  MLIRContext *context,
                                  llvm::StringRef functionName) {
  context->loadDialect<func::FuncDialect>();
  OpBuilder builder(context);

  // Collect data for function creation
  std::vector<Operation *> slice;
  std::vector<Value> values;
  std::vector<Type> outputTypes;

  for (auto *op : ops) {
    // Return op's operands are propagated, but the op itself isn't needed.
    if (!isa<func::ReturnOp>(op))
      slice.push_back(op);

    // All results are returned by the extracted function.
    outputTypes.insert(outputTypes.end(), op->getResults().getTypes().begin(),
                       op->getResults().getTypes().end());

    // Track all values that need to be taken as input to function.
    values.insert(values.end(), op->getOperands().begin(),
                  op->getOperands().end());
  }

  // Create the function
  FunctionType funcType =
      builder.getFunctionType(TypeRange(ValueRange(values)), outputTypes);
  auto loc = builder.getUnknownLoc();
  func::FuncOp funcOp = func::FuncOp::create(loc, functionName, funcType);

  builder.setInsertionPointToEnd(funcOp.addEntryBlock());

  // Map original values to function arguments
  IRMapping mapper;
  for (const auto &arg : llvm::enumerate(values))
    mapper.map(arg.value(), funcOp.getArgument(arg.index()));

  // Clone operations and build function body
  std::vector<Operation *> clonedOps;
  std::vector<Value> clonedVals;
  for (Operation *slicedOp : slice) {
    Operation *clonedOp =
        clonedOps.emplace_back(builder.clone(*slicedOp, mapper));
    clonedVals.insert(clonedVals.end(), clonedOp->result_begin(),
                      clonedOp->result_end());
  }
  // Add return operation
  builder.create<func::ReturnOp>(loc, clonedVals);

  // Remove unused function arguments
  size_t currentIndex = 0;
  while (currentIndex < funcOp.getNumArguments()) {
    if (funcOp.getArgument(currentIndex).use_empty())
      funcOp.eraseArgument(currentIndex);
    else
      ++currentIndex;
  }

  return funcOp;
}

Query::~Query() = default;

LogicalResult InvalidQuery::run(llvm::raw_ostream &os, QuerySession &qs) const {
  os << errStr << "\n";
  return mlir::failure();
}

LogicalResult NoOpQuery::run(llvm::raw_ostream &os, QuerySession &qs) const {
  return mlir::success();
}

LogicalResult HelpQuery::run(llvm::raw_ostream &os, QuerySession &qs) const {
  os << "Available commands:\n\n"
        "  match MATCHER, m MATCHER      "
        "Match the mlir against the given matcher.\n"
        "  quit                              "
        "Terminates the query session.\n\n";
  return mlir::success();
}

LogicalResult QuitQuery::run(llvm::raw_ostream &os, QuerySession &qs) const {
  qs.terminate = true;
  return mlir::success();
}

void collectMatchNodes(
    matcher::BoundOperationNode *Node,
    llvm::SetVector<matcher::BoundOperationNode *> &MatchNodes) {
  MatchNodes.insert(Node);
  for (auto ChildNode : Node->Children) {
    collectMatchNodes(ChildNode, MatchNodes);
  }
}

void analyzeAndPrint(llvm::raw_ostream &os, QuerySession &qs,
                     const matcher::BoundOperationsGraphBuilder &Bound) {

  const auto &Nodes = Bound.getNodes();
  if (Nodes.empty()) {
    os << "The graph is empty.\n";
    return;
  }

  bool AnyDetailedPrinting = false;
  for (const auto &Pair : Nodes) {
    if (Pair.second->DetailedPrinting) {
      AnyDetailedPrinting = true;
      break;
    }
  }

  unsigned MatchesCounter = 0;
  if (!AnyDetailedPrinting) {
    os << "Operations:\n";
    for (const auto &Pair : Nodes) {
      os << "\n";
      os << "  Match #" << ++MatchesCounter << "\n";
      printMatch(os, qs, Pair.first, "root");
    }
    os << MatchesCounter << " matches found!\n";
    return;
  }

  // Maps ids to nodes
  std::unordered_map<Operation *, int> NodeIDs;
  int id = 0;
  for (const auto &Pair : Nodes) {
    NodeIDs[Pair.first] = id++;
  }

  // Finds root nodes
  std::vector<matcher::BoundOperationNode *> RootNodes;
  for (const auto &Pair : Nodes) {
    matcher::BoundOperationNode *Node = Pair.second.get();
    if (Node->IsRootNode) {
      RootNodes.push_back(Node);
    }
  }

  for (auto RootNode : RootNodes) {
    os << "\n";
    os << "  Match #" << ++MatchesCounter << "\n";

    llvm::SetVector<matcher::BoundOperationNode *> MatchNodes;
    collectMatchNodes(RootNode, MatchNodes);
    std::vector<matcher::BoundOperationNode *> SortedMatchNodes(
        MatchNodes.begin(), MatchNodes.end());

    // Sorts based on file location
    std::sort(
        SortedMatchNodes.begin(), SortedMatchNodes.end(),
        [&](matcher::BoundOperationNode *a, matcher::BoundOperationNode *b) {
          auto fileLocA = a->op->getLoc()->findInstanceOf<FileLineColLoc>();
          auto fileLocB = b->op->getLoc()->findInstanceOf<FileLineColLoc>();

          if (!fileLocA && !fileLocB)
            return false;
          if (!fileLocA)
            return false;
          if (!fileLocB)
            return true;

          if (fileLocA.getFilename().str() != fileLocB.getFilename().str())
            return fileLocA.getFilename().str() < fileLocB.getFilename().str();
          return fileLocA.getLine() < fileLocB.getLine();
        });

    for (auto Node : SortedMatchNodes) {
      unsigned NodeID = NodeIDs[Node->op];
      std::string binding = Node->IsRootNode ? "root" : "";
      os << NodeID << ": ";
      printMatch(os, qs, Node->op, binding);
    }

    // Prints edges
    os << "Edges:\n";
    for (auto Node : MatchNodes) {
      int ParentID = NodeIDs[Node->op];
      for (auto ChildNode : Node->Children) {
        if (MatchNodes.count(ChildNode) > 0) {
          int ChildID = NodeIDs[ChildNode->op];
          os << "  " << ParentID << " ---> " << ChildID << "\n";
        }
      }
    }
  }
  os << "\n" << MatchesCounter << " matches found!\n";
}

LogicalResult MatchQuery::run(llvm::raw_ostream &os, QuerySession &qs) const {
  Operation *rootOp = qs.getRootOp();
  int matchCount = 0;
  auto matches = matcher::MatchFinder().getMatches(rootOp, matcher);

  // An extract call is recognized by considering if the matcher has a
  //     name.TODO : Consider making the extract
  //                     more explicit.
  // if (matcher.hasFunctionName()) {
  //   auto functionName = matcher.getFunctionName();
  //   Operation *function = extractFunction(matches.getOperations(),
  //                                         rootOp->getContext(),
  //                                         functionName);
  //   os << "\n" << *function << "\n\n";
  //   function->erase();
  //   return mlir::success();
  // }

  os << "\n";
  analyzeAndPrint(os, qs, matches);

  return mlir::success();
}

} // namespace mlir::query
