#include "CommandAssign.hpp"
#include "driver.hpp"

CommandAssign::CommandAssign(IdentifierPtr const & identifier, ExpressionPtr const & expression)
{
	m_identifier = identifier;
	m_expression = expression;
}


CommandAssign::~CommandAssign()
{
}

std::string CommandAssign::compile(Calculator::Driver & driver) {
	return getCodeBlock(driver)->getRaw();
}

std::string CommandAssign::getCommandName()
{
	return "Command Assign";
}

CodeBlockPtr CommandAssign::getCodeBlock(Calculator::Driver & driver)
{
	CodeBlockPtr codeBlock = std::make_shared<CodeBlock>();

	std::ostringstream compiled;



	compiled << m_expression->evaluateToRegister(driver, 0);

	compiled << m_identifier->loadPositionToRegister(driver, 2);
	compiled << " STORE %r0 %r2 #save expression value to memory\n";
	codeBlock->setSource(compiled);

	return codeBlock;
}

