//=== ParseHLSLRootSignatureTest.cpp - Parse Root Signature tests ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/ModuleLoader.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"

#include "clang/Parse/ParseHLSLRootSignature.h"
#include "gtest/gtest.h"

using namespace llvm::hlsl::root_signature;
using namespace clang;

namespace {

// The test fixture.
class ParseHLSLRootSignatureTest : public ::testing::Test {
protected:
  ParseHLSLRootSignatureTest()
      : FileMgr(FileMgrOpts), DiagID(new DiagnosticIDs()),
        Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
        SourceMgr(Diags, FileMgr), TargetOpts(new TargetOptions) {
    TargetOpts->Triple = "x86_64-apple-darwin11.1.0";
    Target = TargetInfo::CreateTargetInfo(Diags, TargetOpts);
  }

  std::unique_ptr<Preprocessor> CreatePP(StringRef Source,
                                         TrivialModuleLoader &ModLoader) {
    std::unique_ptr<llvm::MemoryBuffer> Buf =
        llvm::MemoryBuffer::getMemBuffer(Source);
    SourceMgr.setMainFileID(SourceMgr.createFileID(std::move(Buf)));

    HeaderSearch HeaderInfo(std::make_shared<HeaderSearchOptions>(), SourceMgr,
                            Diags, LangOpts, Target.get());
    std::unique_ptr<Preprocessor> PP = std::make_unique<Preprocessor>(
        std::make_shared<PreprocessorOptions>(), Diags, LangOpts, SourceMgr,
        HeaderInfo, ModLoader,
        /*IILookup =*/nullptr,
        /*OwnsHeaderSearch =*/false);
    PP->Initialize(*Target);
    PP->EnterMainSourceFile();
    return PP;
  }

  void CheckTokens(SmallVector<RootSignatureToken> &Computed,
                   SmallVector<TokenKind> &Expected) {
    ASSERT_EQ(Computed.size(), Expected.size());
    for (unsigned I = 0, E = Expected.size(); I != E; ++I) {
      ASSERT_EQ(Computed[I].Kind, Expected[I]);
    }
  }

  FileSystemOptions FileMgrOpts;
  FileManager FileMgr;
  IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
  DiagnosticsEngine Diags;
  SourceManager SourceMgr;
  LangOptions LangOpts;
  std::shared_ptr<TargetOptions> TargetOpts;
  IntrusiveRefCntPtr<TargetInfo> Target;
};

TEST_F(ParseHLSLRootSignatureTest, LexValidTokensTest) {
  const llvm::StringLiteral Source = R"cc(
    -42

    b0 t43 u987 s234

    (),|=

    DescriptorTable

    CBV SRV UAV Sampler
    space visibility flags
    numDescriptors offset

    DESCRIPTOR_RANGE_OFFSET_APPEND

    DATA_VOLATILE
    DATA_STATIC_WHILE_SET_AT_EXECUTE
    DATA_STATIC
    DESCRIPTORS_VOLATILE
    DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS

    shader_visibility_all
    shader_visibility_vertex
    shader_visibility_hull
    shader_visibility_domain
    shader_visibility_geometry
    shader_visibility_pixel
    shader_visibility_amplification
    shader_visibility_mesh

    42 +42
  )cc";

  TrivialModuleLoader ModLoader;
  auto PP = CreatePP(Source, ModLoader);
  auto TokLoc = SourceLocation();

  RootSignatureLexer Lexer(Source, TokLoc, *PP);

  SmallVector<RootSignatureToken> Tokens = {
      RootSignatureToken() // invalid token for completeness
  };
  ASSERT_FALSE(Lexer.Lex(Tokens));

  SmallVector<TokenKind> Expected = {
#define TOK(NAME) TokenKind::NAME,
#include "clang/Parse/HLSLRootSignatureTokenKinds.def"
      TokenKind::int_literal,
      TokenKind::int_literal,
  };

  CheckTokens(Tokens, Expected);
}

} // anonymous namespace
