//===--- ParseHLSLRootSignature.h -------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the ParseHLSLRootSignature interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_PARSE_PARSEHLSLROOTSIGNATURE_H
#define LLVM_CLANG_PARSE_PARSEHLSLROOTSIGNATURE_H

#include "clang/Lex/LiteralSupport.h"
#include "clang/Lex/Preprocessor.h"

#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"

#include "llvm/Frontend/HLSL/HLSLRootSignature.h"

namespace llvm {
namespace hlsl {
namespace root_signature {

struct RootSignatureToken {
  enum Kind {
#define TOK(X) X,
#include "clang/Parse/HLSLRootSignatureTokenKinds.def"
  };

  Kind Kind = Kind::invalid;

  // Retain the SouceLocation of the token for diagnostics
  clang::SourceLocation TokLoc;

  // Retain if the uint32_t bits represent a signed integer
  bool Signed = false;
  union {
    uint32_t IntLiteral = 0;
    float FloatLiteral;
  };

  // Constructors
  RootSignatureToken() {}
  RootSignatureToken(clang::SourceLocation TokLoc) : TokLoc(TokLoc) {}
};
using TokenKind = enum RootSignatureToken::Kind;

class RootSignatureLexer {
public:
  RootSignatureLexer(StringRef Signature, clang::SourceLocation SourceLoc,
                     clang::Preprocessor &PP)
      : Buffer(Signature), SourceLoc(SourceLoc), PP(PP) {}

  // Consumes the internal buffer as a list of tokens and will emplace them
  // onto the given tokens.
  //
  // It will consume until it successfully reaches the end of the buffer,
  // or, until the first error is encountered. The return value denotes if
  // there was a failure.
  bool Lex(SmallVector<RootSignatureToken> &Tokens);

  // Get the current source location of the lexer
  clang::SourceLocation GetLocation() { return SourceLoc; };

private:
  // Internal buffer to iterate over
  StringRef Buffer;

  // Passed down parameters from Sema
  clang::SourceLocation SourceLoc;
  clang::Preprocessor &PP;

  bool LexNumber(RootSignatureToken &Result);

  // Consumes the internal buffer for a single token.
  //
  // The return value denotes if there was a failure.
  bool LexToken(RootSignatureToken &Token);

  // Advance the buffer by the specified number of characters. Updates the
  // SourceLocation appropriately.
  void AdvanceBuffer(unsigned NumCharacters = 1) {
    Buffer = Buffer.drop_front(NumCharacters);
    SourceLoc = SourceLoc.getLocWithOffset(NumCharacters);
  }
};

class RootSignatureParser {
public:
  RootSignatureParser(SmallVector<RootElement> &Elements,
                      const SmallVector<RootSignatureToken> &Tokens);

  // Iterates over the provided tokens and constructs the in-memory
  // representations of the RootElements.
  //
  // The return value denotes if there was a failure and the method will
  // return on the first encountered failure, or, return false if it
  // can sucessfully reach the end of the tokens.
  bool Parse();

private:
  bool ReportError(); // TODO: Implement this to report error through Diags

  // Root Element helpers
  bool ParseRootElement();
  bool ParseDescriptorTable();
  bool ParseDescriptorTableClause();

  // Common parsing helpers
  bool ParseRegister(Register &Register);

  // Various flags/enum parsing helpers
  bool ParseDescriptorRangeFlags(DescriptorRangeFlags &Flags);
  bool ParseShaderVisibility(ShaderVisibility &Flag);

  // Increment the token iterator if we have not reached the end.
  // Return value denotes if we were already at the last token.
  bool ConsumeNextToken();

  // Attempt to retrieve the next token, if TokenKind is invalid then there was
  // no next token.
  RootSignatureToken PeekNextToken();

  // Peek if the next token is of the expected kind.
  //
  // Return value denotes if it failed to match the expected kind, either it is
  // the end of the stream or it didn't match any of the expected kinds.
  bool PeekExpectedToken(TokenKind Expected);
  bool PeekExpectedToken(ArrayRef<TokenKind> AnyExpected);

  // Consume the next token and report an error if it is not of the expected
  // kind.
  //
  // Return value denotes if it failed to match the expected kind, either it is
  // the end of the stream or it didn't match any of the expected kinds.
  bool ConsumeExpectedToken(TokenKind Expected);
  bool ConsumeExpectedToken(ArrayRef<TokenKind> AnyExpected);

  // Peek if the next token is of the expected kind and if it is then consume
  // it.
  //
  // Return value denotes if it failed to match the expected kind, either it is
  // the end of the stream or it didn't match any of the expected kinds. It will
  // not report an error if there isn't a match.
  bool TryConsumeExpectedToken(TokenKind Expected);
  bool TryConsumeExpectedToken(ArrayRef<TokenKind> Expected);

private:
  SmallVector<RootElement> &Elements;
  SmallVector<RootSignatureToken>::const_iterator CurTok;
  SmallVector<RootSignatureToken>::const_iterator LastTok;
};

} // namespace root_signature
} // namespace hlsl
} // namespace llvm

#endif // LLVM_CLANG_PARSE_PARSEHLSLROOTSIGNATURE_H
