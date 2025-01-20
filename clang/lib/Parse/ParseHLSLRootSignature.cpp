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

} // namespace root_signature
} // namespace hlsl
} // namespace llvm
