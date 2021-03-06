#include <cctype>
#include <fstream>
#include <cassert>
#include <sstream>
#include <algorithm>

#include <iostream>

#include "driver.hpp"
#include "CodeBlock.hpp"
#include "GebalaCompiler.hpp"
#include "logging.h"

Calculator::Driver::~Driver(){
  delete(scanner);
  scanner = nullptr;
  delete(parser);
  parser = nullptr;
}

void Calculator::Driver::parse( std::string input )
{
  LOG("input:\n" << input << "\nend input");

  std::istringstream inputStream(input);
  m_errorStream.str("");

  m_lastLabelNumbers.clear();
  m_lastMemoryAddress = 0;

  delete(scanner);
  try
  {
    scanner = new Calculator::Scanner( &inputStream );
  }
  catch( std::bad_alloc &ba )
  {
    m_errorStream << "Failed to allocate scanner: (" <<
    ba.what() << ")\n";
    return;
  }

  delete(parser);
  try
  {
    parser = new Calculator::Parser( (*scanner) /* scanner */,
    (*this) /* driver */ );
  }
  catch( std::bad_alloc &ba )
  {
    m_errorStream << "Failed to allocate parser: (" <<
    ba.what() << "\n";
    return;
  }

  parser->parse();
}

void Calculator::Driver::addError(std::string const & error)
{
  m_errorStream << error;
}

std::string Calculator::Driver::getError() const
{
  return m_errorStream.str();
}

void Calculator::Driver::setCommands(CommandPtrs const & commands) {
  LOG("Set commands: " << commands.size());
  m_commands = commands;
}

bool Calculator::Driver::isError() const
{
  return m_errorStream.str() != "";
}

CodeBlockPtr Calculator::Driver::compileToBlock() {
  CodeBlockPtr codeBlock = std::make_shared<CodeBlock>();
  LOG(m_commands.size());
  for (CommandPtr cmd : m_commands) {
    LOG(cmd->getCommandName());
    codeBlock->addBlock(cmd->getCodeBlock(*this));
  }
  return codeBlock;
}

std::ostream& Calculator::Driver::compile(std::ostream &stream)
{
  std::string compiled = compileToBlock()->getRaw();
  stream << GebalaCompiler::compile(compiled);
  return stream;
}


std::string Calculator::Driver::getNextLabelFor(std::string labelName) {
  std::ostringstream stringStream;
  auto labelNumber = m_lastLabelNumbers[labelName];
  stringStream << "label_" << labelName << "_" << labelNumber;

  m_lastLabelNumbers[labelName] = labelNumber + 1;

  return stringStream.str();
}

std::string Calculator::Driver::getUniqueNameFor(std::string name)
{
  static int currentNumber = 0;
  std::ostringstream stringStream;
  stringStream << "tmp_" << name << "_" << currentNumber++;

  return stringStream.str();
}


bool Calculator::Driver::doesDeclarationExist(std::string identifierName)
{
  LOG("check: " << identifierName << " from elems: " << m_declaredVariables.size());
  if(m_declaredVariables.empty()) return false;
  else return m_declaredVariables.find(identifierName) != std::end(m_declaredVariables);
}

void Calculator::Driver::undeclare(std::string identifierName)
{
  m_declaredVariables.erase(identifierName);
}

Number Calculator::Driver::getDeclarationPosition(std::string identifierName)
{
  if (!doesDeclarationExist(identifierName)) {
    //TODO implement not declared handling
    std::cerr << "Undeclared variable " << identifierName << std::endl;
    return Number();
  }
  return m_declaredVariables[identifierName];
}

void Calculator::Driver::declare(std::string identifierName) {
  declare(identifierName, Number(1));
}

void Calculator::Driver::declare(std::string identifierName, Number size) {
  if (doesDeclarationExist(identifierName)) {
    //TODO implement redeclaration handling
    std::cerr << "Redeclaration of " << identifierName << std::endl;
    return;
  }
  m_declaredVariables[identifierName] = m_lastMemoryAddress;
  LOG("declare: " << identifierName << " with size: " << size << " @ " << m_lastMemoryAddress);
  m_lastMemoryAddress += size;
}

Number Calculator::Driver::getTmpMemoryPosition()
{
  return m_lastMemoryAddress + 1;
}
