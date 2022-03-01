# IPP project, task 2, part 1: interpret.py
# File:   exceptions.py
# Author: Ondřej Ondryáš (xondry02@stud.fit.vubr.cz)
# Date:   2021-04-11

from typing import Optional, Tuple

InterpreterContext = 'InterpreterContext'  # For sake of typing hints
Instruction = 'Instruction'


class SourceNotLoadedError(Exception):
    def __init__(self, *args):
        super().__init__('Cannot start the interpreter before loading a source file', *args)


class XmlMalformedError(Exception):
    def __init__(self, position: Optional[Tuple[int, int]], *args):
        super().__init__(f"Source XML is malformed (line {position[0]}, column {position[1]})", *args)
        self.exit_code = 31


class XmlProgramFormatError(Exception):
    def __init__(self, message: str, *args):
        super().__init__(*args, 'Unexpected XML structure: ' + message)
        self.exit_code = 32


class InvalidOpcodeError(Exception):
    def __init__(self, opcode: str, instruction_number: int, *args):
        super().__init__(instruction_number, opcode, f"Unknown instruction code '{opcode}'", *args)
        self.exit_code = 32
        self.opcode = opcode
        self.instruction_number = instruction_number


class LexicalError(Exception):
    def __init__(self, invalid_literal: str, expected_type: str, opcode: Optional[str] = None,
                 instruction_number: Optional[int] = None, *args):
        super().__init__(instruction_number, opcode, f"Invalid {expected_type} literal: '{invalid_literal}'",
                         *args)
        self.exit_code = 32
        self.instruction_number = instruction_number
        self.expected_type = expected_type
        self.invalid_literal = invalid_literal


class InterpretError(Exception):
    def __init__(self, context: Optional[InterpreterContext], current_instruction: Optional[Instruction], *args):
        args_list = list(args)
        if context is not None:
            args_list.append(context.program_counter)
        if current_instruction is not None:
            args_list.append(current_instruction.name)
        args_list = tuple(args_list)

        super().__init__(*args_list)
        self.instruction = current_instruction
        if context is not None:
            self.instruction_number = context.program_counter
        self.context = context
        self.exit_code = 52


class RedefinedVariableError(InterpretError):
    def __init__(self, context: Optional[InterpreterContext], current_instruction: Optional[Instruction], var_name: str,
                 *args):
        super().__init__(context, current_instruction, 'Variable is already defined', var_name, *args)


class RedefinedLabelError(InterpretError):
    def __init__(self, context: Optional[InterpreterContext], current_instruction: Optional[Instruction], label_name: str,
                 *args):
        super().__init__(context, current_instruction, 'Label is already defined', label_name, *args)


class UndefinedLabelError(InterpretError):
    def __init__(self, context: Optional[InterpreterContext], current_instruction: Optional[Instruction], *args):
        super().__init__(context, current_instruction, 'Label is not defined', *args)


class OperandException(InterpretError):
    def __init__(self, context: Optional[InterpreterContext], current_instruction: Optional[Instruction], *args):
        super().__init__(context, current_instruction, 'Invalid operand', *args)
        self.exit_code = 53


class UndefinedVariableError(InterpretError):
    def __init__(self, context: Optional[InterpreterContext], current_instruction: Optional[Instruction], var_name: str,
                 *args):
        super().__init__(context, current_instruction, 'Variable is not defined', var_name, *args)
        self.exit_code = 54


class UndefinedFrameError(InterpretError):
    def __init__(self, context: Optional[InterpreterContext], current_instruction: Optional[Instruction], frame_name: str,
                 *args):
        super().__init__(context, current_instruction, 'Frame is not defined', frame_name, *args)
        self.exit_code = 55


class VariableMissingValueError(InterpretError):
    def __init__(self, context: Optional[InterpreterContext], current_instruction: Optional[Instruction], var_name: str,
                 *args):
        super().__init__(context, current_instruction, 'Variable has no value', var_name, *args)
        self.exit_code = 56


class StackMissingValueError(InterpretError):
    def __init__(self, context: Optional[InterpreterContext], current_instruction: Optional[Instruction], *args):
        super().__init__(context, current_instruction, 'Not enough values on the variable stack', *args)
        self.exit_code = 56


class CallStackEmptyError(InterpretError):
    def __init__(self, context: Optional[InterpreterContext], current_instruction: Optional[Instruction], *args):
        super().__init__(context, current_instruction, 'The call stack is empty, invalid RETURN', *args)
        self.exit_code = 56


class InvalidValueError(InterpretError):
    def __init__(self, context: Optional[InterpreterContext], current_instruction: Optional[Instruction], message: str,
                 *args):
        super().__init__(context, current_instruction, message, *args)
        self.exit_code = 57


class InvalidStringOperationError(InterpretError):
    def __init__(self, context: Optional[InterpreterContext], current_instruction: Optional[Instruction], message: str,
                 *args):
        super().__init__(context, current_instruction, message, *args)
        self.exit_code = 58


class InvalidTypeError(Exception):
    pass


class ArgumentDefinitionError(Exception):
    def __init__(self):
        super().__init__('Internal error: invalid instruction argument definition')
