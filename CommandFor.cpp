#include "CommandFor.hpp"
#include "driver.hpp"

#include "CommandWhile.hpp"
#include "CommandAssign.hpp"
#include "Condition.hpp"
#include "ExpressionNumber.hpp"
#include "ExpressionOperation.hpp"
#include "ExpressionIdentifier.hpp"

CommandFor::CommandFor(Identifier::IdentifierName identifierName,
	IdentifierPtr fromIdentifier,
	IdentifierPtr toIdentifier,
	CommandPtrs const & commands,
	bool downFor)
{
	m_operands = Operands::IDENTIFIER_IDENTIFIER;
	m_identifierName = identifierName;
	m_fromIdentifier = fromIdentifier;
	m_toIdentifier = toIdentifier;
	m_commands = commands;
	m_isDownFor = downFor;
}

CommandFor::CommandFor(Identifier::IdentifierName identifierName,
	IdentifierPtr fromIdentifier,
	Number toNumber,
	CommandPtrs const & commands,
	bool downFor)
{
	m_operands = Operands::IDENTIFIER_NUMBER;
	m_identifierName = identifierName;
	m_fromIdentifier = fromIdentifier;
	m_toNumber = toNumber;
	m_commands = commands;
	m_isDownFor = downFor;
}

CommandFor::CommandFor(Identifier::IdentifierName identifierName,
	Number fromNumber,
	IdentifierPtr toIdentifier,
	CommandPtrs const & commands,
	bool downFor)
{
	m_operands = Operands::NUMBER_IDENTIFIER;
	m_identifierName = identifierName;
	m_fromNumber = fromNumber;
	m_toIdentifier = toIdentifier;
	m_commands = commands;
	m_isDownFor = downFor;
}

CommandFor::CommandFor(Identifier::IdentifierName identifierName,
	Number fromNumber,
	Number toNumber,
	CommandPtrs const & commands,
	bool downFor)
{
	m_operands = Operands::NUMBER_NUMBER;
	m_identifierName = identifierName;
	m_fromNumber = fromNumber;
	m_toNumber = toNumber;
	m_commands = commands;
	m_isDownFor = downFor;
}

CommandFor::~CommandFor()
{
}

std::string CommandFor::compile(Calculator::Driver & driver) {
	std::ostringstream compiled;

	driver.declare(m_identifierName);
	auto identifier = std::make_shared<Identifier>(m_identifierName);

	compiled << "### begin of 'for' block\n";


  ExpressionPtr initialAssignExpression;


	switch(m_operands)
	{
		case Operands::IDENTIFIER_IDENTIFIER:
		case Operands::IDENTIFIER_NUMBER:
		  compiled << "#" << m_identifierName << " := " << m_fromIdentifier->getName() << "\n";
		  initialAssignExpression = std::make_shared<ExpressionIdentifier>(m_fromIdentifier);
		break;
		case Operands::NUMBER_IDENTIFIER:
		case Operands::NUMBER_NUMBER:
		  compiled << "#" << m_identifierName << " := " << m_fromNumber << "\n";
		  initialAssignExpression = std::make_shared<ExpressionNumber>(m_fromNumber);
		break;
	}
  auto initialAssign = std::make_shared<CommandAssign>(identifier, initialAssignExpression);

	auto tmpToIdentifierName = driver.getUniqueNameFor("tmpTo");
	driver.declare(tmpToIdentifierName);
	auto tmpToIdentifier = std::make_shared<Identifier>(tmpToIdentifierName);

  ConditionPtr condition;
	switch(m_operands)
	{
		case Operands::IDENTIFIER_IDENTIFIER:
		case Operands::NUMBER_IDENTIFIER:
		  if(m_isDownFor)
			{
				compiled << "# WHILE " << m_identifierName << " > " << tmpToIdentifier->getName() << " DO \n";
				compiled << "# {commands}\n";
				compiled << "#" << m_identifierName << " := " << m_identifierName << " - 1\n";
				compiled << "# ENDWHILE \n";

				condition = std::make_shared<Condition>(Condition::Type::OP_GT, identifier, tmpToIdentifier);
			}
			else
			{
				compiled << "# WHILE " <<  tmpToIdentifier->getName() << " > " << m_identifierName << " DO \n";
				compiled << "# {commands}\n";
				compiled << "#" << m_identifierName << " := " << m_identifierName << " + 1\n";
				compiled << "# ENDWHILE \n";

				condition = std::make_shared<Condition>(Condition::Type::OP_GE, tmpToIdentifier, identifier);
			}
		break;
		case Operands::IDENTIFIER_NUMBER:
		case Operands::NUMBER_NUMBER:
		  if(m_isDownFor)
			{
				compiled << "# WHILE " << m_identifierName << " > " << m_toNumber << " DO \n";
				compiled << "# {commands}\n";
				compiled << "#" << m_identifierName << " := " << m_identifierName << " - 1\n";
				compiled << "# ENDWHILE \n";

				condition = std::make_shared<Condition>(Condition::Type::OP_GT, identifier, m_toNumber);
			}
			else
			{
				compiled << "# WHILE " <<  m_toNumber << " > " << m_identifierName << " DO \n";
				compiled << "# {commands}\n";
				compiled << "#" << m_identifierName << " := " << m_identifierName << " + 1\n";
				compiled << "# ENDWHILE \n";

				condition = std::make_shared<Condition>(Condition::Type::OP_GE, m_toNumber, identifier);
			}
		break;
	}

	auto commands = m_commands;
  auto stepAssign = std::make_shared<CommandAssign>( identifier, std::make_shared<ExpressionOperation>((m_isDownFor ? ExpressionOperation::Type::OP_SUB : ExpressionOperation::Type::OP_ADD), identifier, 1) );
	commands.push_back(stepAssign);

	auto whileCmd = std::make_shared<CommandWhile>(condition, commands);

  compiled << initialAssign->compile(driver);

	if(m_operands == Operands::IDENTIFIER_IDENTIFIER || m_operands == Operands::NUMBER_IDENTIFIER)
	{
		auto initialAssignTmpToIdentifier = std::make_shared<CommandAssign>(tmpToIdentifier, std::make_shared<ExpressionIdentifier>(m_toIdentifier));
		compiled << "# copy to identifier to tmp identifier\n";
		compiled << initialAssignTmpToIdentifier->compile(driver);
	}

	compiled << whileCmd->compile(driver);
	if(m_isDownFor)
	{
		for(auto& cmd : commands)
		{
			compiled << cmd->compile(driver);
		}
	}

	compiled << "### end of 'for' block\n";

	driver.undeclare(m_identifierName);
	driver.undeclare(tmpToIdentifierName);

	return compiled.str();
}

std::string CommandFor::getCommandName()
{
	return "Command For";
}

CodeBlockPtr CommandFor::getCodeBlock(Calculator::Driver & driver)
{
	CodeBlockPtr codeBlock = std::make_shared<CodeBlock>();

	codeBlock->setSource(compile(driver));

	return codeBlock;
}
