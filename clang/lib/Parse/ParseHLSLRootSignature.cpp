#include "clang/Parse/ParseHLSLRootSignature.h"

namespace llvm {
namespace hlsl {
namespace root_signature {

// Lexer Definitions

static bool IsNumberChar(char C) {
  // TODO: extend for float support with or without hexadecimal/exponent
  return isdigit(C); // integer support
}

bool RootSignatureLexer::LexNumber(RootSignatureToken &Result) {
  // NumericLiteralParser does not handle the sign so we will manually apply it
  bool Negative = Buffer.front() == '-';
  Result.Signed = Negative || Buffer.front() == '+';
  if (Result.Signed)
    AdvanceBuffer();

  // Retrieve the possible number
  StringRef NumSpelling = Buffer.take_while(IsNumberChar);

  // Parse the numeric value and do semantic checks on its specification
  clang::NumericLiteralParser Literal(NumSpelling, SourceLoc,
                                      PP.getSourceManager(), PP.getLangOpts(),
                                      PP.getTargetInfo(), PP.getDiagnostics());
  if (Literal.hadError)
    return true; // Error has already been reported so just return

  if (!Literal.isIntegerLiteral())
    return true; // TODO: report unsupported number literal specification

  // Retrieve the number value to store into the token
  Result.Kind = TokenKind::int_literal;

  APSInt X = APSInt(32, Result.Signed);
  if (Literal.GetIntegerValue(X))
    return true; // TODO: Report overflow error

  X = Negative ? -X : X;
  Result.IntLiteral = (uint32_t)X.getZExtValue();

  AdvanceBuffer(NumSpelling.size());
  return false;
}

bool RootSignatureLexer::Lex(SmallVector<RootSignatureToken> &Tokens) {
  // Discard any leading whitespace
  AdvanceBuffer(Buffer.take_while(isspace).size());

  while (!Buffer.empty()) {
    RootSignatureToken Result;
    if (LexToken(Result))
      return true;

    // Successfully Lexed the token so we can store it
    Tokens.push_back(Result);

    // Discard any trailing whitespace
    AdvanceBuffer(Buffer.take_while(isspace).size());
  }

  return false;
}

bool RootSignatureLexer::LexToken(RootSignatureToken &Result) {
  // Record where this token is in the text for usage in parser diagnostics
  Result.TokLoc = SourceLoc;

  char C = Buffer.front();

  // Punctuators
  switch (C) {
#define PUNCTUATOR(X, Y)                                                       \
  case Y: {                                                                    \
    Result.Kind = TokenKind::pu_##X;                                           \
    AdvanceBuffer();                                                           \
    return false;                                                              \
  }
#include "clang/Parse/HLSLRootSignatureTokenKinds.def"
  default:
    break;
  }

  // Numeric constant
  if (isdigit(C) || C == '-' || C == '+')
    return LexNumber(Result);

  // All following tokens require at least one additional character
  if (Buffer.size() <= 1)
    return true; // TODO: Report invalid token error

  // Peek at the next character to deteremine token type
  char NextC = Buffer[1];

  // Registers: [tsub][0-9+]
  if ((C == 't' || C == 's' || C == 'u' || C == 'b') && isdigit(NextC)) {
    AdvanceBuffer();

    if (LexNumber(Result))
      return true;

    // Lex number could also parse a signed int/float so ensure it was an
    // unsigned int
    if (Result.Kind != TokenKind::int_literal || Result.Signed)
      return true; // Return invalid number literal for register error

    // Convert character to the register type.
    // This is done after LexNumber to override the TokenKind
    switch (C) {
    case 'b':
      Result.Kind = TokenKind::bReg;
      break;
    case 't':
      Result.Kind = TokenKind::tReg;
      break;
    case 'u':
      Result.Kind = TokenKind::uReg;
      break;
    case 's':
      Result.Kind = TokenKind::sReg;
      break;
    default:
      llvm_unreachable("Switch for an expected token was not provided");
      return true;
    }
    return false;
  }

  // Keywords and Enums:
  StringRef TokSpelling =
      Buffer.take_while([](char C) { return isalnum(C) || C == '_'; });

  // Define a large string switch statement for all the keywords and enums
  auto Switch = llvm::StringSwitch<TokenKind>(TokSpelling);
#define KEYWORD(NAME) Switch.Case(#NAME, TokenKind::kw_##NAME);
#define ENUM(NAME, LIT) Switch.CaseLower(LIT, TokenKind::en_##NAME);
#include "clang/Parse/HLSLRootSignatureTokenKinds.def"

  // Then attempt to retreive a string from it
  auto Kind = Switch.Default(TokenKind::invalid);
  if (Kind == TokenKind::invalid)
    return true; // TODO: Report invalid identifier

  Result.Kind = Kind;
  AdvanceBuffer(TokSpelling.size());
  return false;
}

// Parser Definitions

RootSignatureParser::RootSignatureParser(
    SmallVector<RootElement> &Elements,
    const SmallVector<RootSignatureToken> &Tokens)
    : Elements(Elements) {
  CurTok = Tokens.begin();
  LastTok = Tokens.end();
}

bool RootSignatureParser::ReportError() { return true; }

bool RootSignatureParser::Parse() {
  CurTok--; // Decrement once here so we can use the ...ExpectedToken api

  // Iterate as many RootElements as possible
  bool HasComma = true;
  while (HasComma &&
         !TryConsumeExpectedToken(ArrayRef{TokenKind::kw_DescriptorTable})) {
    if (ParseRootElement())
      return true;
    HasComma = !TryConsumeExpectedToken(TokenKind::pu_comma);
  }
  if (HasComma)
    return ReportError(); // report 'comma' denotes a required extra item

  // Ensure that we are at the end of the tokens
  CurTok++;
  if (CurTok != LastTok)
    return ReportError(); // report expected end of input but got more
  return false;
}

bool RootSignatureParser::ParseRootElement() {
  // Dispatch onto the correct parse method
  switch (CurTok->Kind) {
  case TokenKind::kw_DescriptorTable:
    return ParseDescriptorTable();
  default:
    llvm_unreachable("Switch for an expected token was not provided");
    return true;
  }
}

bool RootSignatureParser::ParseDescriptorTable() {
  DescriptorTable Table;

  if (ConsumeExpectedToken(TokenKind::pu_l_paren))
    return true;

  // Iterate as many DescriptorTableClaues as possible
  bool HasComma = true;
  while (!TryConsumeExpectedToken({TokenKind::kw_CBV, TokenKind::kw_SRV,
                                   TokenKind::kw_UAV, TokenKind::kw_Sampler})) {
    if (ParseDescriptorTableClause())
      return true;
    Table.NumClauses++;
    HasComma = !TryConsumeExpectedToken(TokenKind::pu_comma);
  }

  // Consume optional 'visibility' paramater
  if (HasComma && !TryConsumeExpectedToken(TokenKind::kw_visibility)) {
    if (ConsumeExpectedToken(TokenKind::pu_equal))
      return true;

    if (ParseShaderVisibility(Table.Visibility))
      return true;

    HasComma = !TryConsumeExpectedToken(TokenKind::pu_comma);
  }

  if (HasComma && Table.NumClauses != 0)
    return ReportError(); // report 'comma' denotes a required extra item

  if (ConsumeExpectedToken(TokenKind::pu_r_paren))
    return true;

  Elements.push_back(RootElement(Table));
  return false;
}

bool RootSignatureParser::ParseDescriptorTableClause() {
  // Determine the type of Clause first so we can initialize the struct with
  // the correct default flags
  ClauseType CT;
  switch (CurTok->Kind) {
  case TokenKind::kw_CBV:
    CT = ClauseType::CBV;
    break;
  case TokenKind::kw_SRV:
    CT = ClauseType::SRV;
    break;
  case TokenKind::kw_UAV:
    CT = ClauseType::UAV;
    break;
  case TokenKind::kw_Sampler:
    CT = ClauseType::Sampler;
    break;
  default:
    llvm_unreachable("Switch for an expected token was not provided");
    return true;
  }
  DescriptorTableClause Clause(CT);

  if (ConsumeExpectedToken(TokenKind::pu_l_paren))
    return true;

  // Consume mandatory Register paramater
  if (ConsumeExpectedToken(
          {TokenKind::bReg, TokenKind::tReg, TokenKind::uReg, TokenKind::sReg}))
    return true;
  if (ParseRegister(Clause.Register))
    return true;

  // Start parsing the optional parameters
  bool HasComma = !TryConsumeExpectedToken(TokenKind::pu_comma);

  // Consume optional 'numDescriptors' paramater
  if (HasComma && !TryConsumeExpectedToken(TokenKind::kw_numDescriptors)) {
    if (ConsumeExpectedToken(TokenKind::pu_equal))
      return true;
    if (ConsumeExpectedToken(TokenKind::int_literal))
      return true;

    Clause.NumDescriptors = CurTok->IntLiteral;

    HasComma = !TryConsumeExpectedToken(TokenKind::pu_comma);
  }

  // Consume optional 'space' paramater
  if (HasComma && !TryConsumeExpectedToken(TokenKind::kw_space)) {
    if (ConsumeExpectedToken(TokenKind::pu_equal))
      return true;
    if (ConsumeExpectedToken(TokenKind::int_literal))
      return true;

    Clause.Space = CurTok->IntLiteral;

    HasComma = !TryConsumeExpectedToken(TokenKind::pu_comma);
  }

  // Consume optional 'offset' paramater
  if (HasComma && !TryConsumeExpectedToken(TokenKind::kw_offset)) {
    if (ConsumeExpectedToken(TokenKind::pu_equal))
      return true;
    if (ConsumeExpectedToken(ArrayRef{
            TokenKind::int_literal, TokenKind::en_DescriptorRangeOffsetAppend}))
      return true;

    // Offset defaults to DescriptorTableOffsetAppend so only change if we have
    // an int arg
    if (CurTok->Kind == TokenKind::int_literal)
      Clause.Offset = CurTok->IntLiteral;

    HasComma = !TryConsumeExpectedToken(TokenKind::pu_comma);
  }

  // Consume optional 'flags' paramater
  if (HasComma && !TryConsumeExpectedToken(TokenKind::kw_flags)) {
    if (ConsumeExpectedToken(TokenKind::pu_equal))
      return true;
    if (ParseDescriptorRangeFlags(Clause.Flags))
      return true;

    HasComma = !TryConsumeExpectedToken(TokenKind::pu_comma);
  }

  if (HasComma)
    return ReportError(); // report 'comma' denotes a required extra item
  if (ConsumeExpectedToken(TokenKind::pu_r_paren))
    return true;

  Elements.push_back(Clause);
  return false;
}

bool RootSignatureParser::ParseRegister(Register &Register) {
  switch (CurTok->Kind) {
  case TokenKind::bReg:
    Register.ViewType = RegisterType::BReg;
    break;
  case TokenKind::tReg:
    Register.ViewType = RegisterType::TReg;
    break;
  case TokenKind::uReg:
    Register.ViewType = RegisterType::UReg;
    break;
  case TokenKind::sReg:
    Register.ViewType = RegisterType::SReg;
    break;
  default:
    llvm_unreachable("Switch for an expected token was not provided");
    return true;
  }

  Register.Number = CurTok->IntLiteral;

  return false;
}

bool RootSignatureParser::ParseDescriptorRangeFlags(
    DescriptorRangeFlags &Flags) {

  // Define the possible flag kinds
  SmallVector<TokenKind> FlagToks = {
      TokenKind::int_literal, // This is used to capture the possible '0'
#define DESCRIPTOR_RANGE_FLAG_ENUM(NAME, LIT, ON) TokenKind::en_##NAME,
#include "clang/Parse/HLSLRootSignatureTokenKinds.def"
  };

  if (PeekExpectedToken(FlagToks))
    return ReportError(); // report there must be at least one flag specified

  // Since there is at least one flag specified then reset the default flag
  Flags = DescriptorRangeFlags::None;

  // Iterate over the given list of flags
  bool HasOr = true;
  while (HasOr && !TryConsumeExpectedToken(FlagToks)) {
    switch (CurTok->Kind) {
    case TokenKind::int_literal: {
      if (CurTok->IntLiteral != 0)
        return ReportError(); // report invalid flag value error
      // No need to 'or' with 0 so just break
      break;
    }
    // Set each specified flag set in the flags
#define DESCRIPTOR_RANGE_FLAG_ENUM(NAME, LIT, ON)                              \
  case TokenKind::en_##NAME: {                                                 \
    Flags |= DescriptorRangeFlags::NAME;                                       \
    break;                                                                     \
  }
#include "clang/Parse/HLSLRootSignatureTokenKinds.def"
    default:
      llvm_unreachable("Switch for an expected token was not provided");
      return true;
    }
    HasOr = !TryConsumeExpectedToken(TokenKind::pu_or);
  }
  if (HasOr)
    return ReportError(); // report 'or' denotes a required extra item

  return false;
}

bool RootSignatureParser::ParseShaderVisibility(ShaderVisibility &Flag) {

  // Define the possible flag kinds
  SmallVector<TokenKind> FlagToks = {
#define SHADER_VISIBILITY_ENUM(NAME, LIT) TokenKind::en_##NAME,
#include "clang/Parse/HLSLRootSignatureTokenKinds.def"
  };

  // Required mandatory flag argument
  if (ConsumeExpectedToken(FlagToks))
    return true;

  switch (CurTok->Kind) {
#define SHADER_VISIBILITY_ENUM(NAME, LIT)                                      \
  case TokenKind::en_##NAME: {                                                 \
    Flag = ShaderVisibility::NAME;                                             \
    break;                                                                     \
  }
#include "clang/Parse/HLSLRootSignatureTokenKinds.def"
  default:
    llvm_unreachable("Switch for an expected token was not provided");
    return true;
  }

  return false;
}

RootSignatureToken RootSignatureParser::PeekNextToken() {
  RootSignatureToken Token; // Defaults to invalid kind
  if (CurTok != LastTok)
    Token = *(CurTok + 1);
  return Token;
}

bool RootSignatureParser::ConsumeNextToken() {
  if (CurTok == LastTok)
    return ReportError(); // Report unexpected end of tokens error
  CurTok++;
  return false;
}

bool RootSignatureParser::PeekExpectedToken(TokenKind Expected) {
  return PeekExpectedToken(ArrayRef{Expected});
}

bool RootSignatureParser::PeekExpectedToken(ArrayRef<TokenKind> AnyExpected) {
  RootSignatureToken Token = PeekNextToken();
  if (Token.Kind == TokenKind::invalid)
    return true;
  for (auto Expected : AnyExpected) {
    if (Token.Kind == Expected)
      return false;
  }
  return true;
}

bool RootSignatureParser::ConsumeExpectedToken(TokenKind Expected) {
  return ConsumeExpectedToken(ArrayRef{Expected});
}

bool RootSignatureParser::ConsumeExpectedToken(
    ArrayRef<TokenKind> AnyExpected) {
  if (ConsumeNextToken())
    return true;
  for (auto Expected : AnyExpected) {
    if (CurTok->Kind == Expected)
      return false;
  }
  return ReportError(); // Report unexpected token kind error
}

bool RootSignatureParser::TryConsumeExpectedToken(TokenKind Expected) {
  return TryConsumeExpectedToken(ArrayRef{Expected});
}

bool RootSignatureParser::TryConsumeExpectedToken(
    ArrayRef<TokenKind> AnyExpected) {
  if (PeekExpectedToken(AnyExpected))
    return true;
  return ConsumeNextToken();
}

} // namespace root_signature
} // namespace hlsl
} // namespace llvm
