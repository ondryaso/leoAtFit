# IPP project, task 2, part 1: interpret.py
# File:   instruction_definitions.py
# Author: Ondřej Ondryáš (xondry02@stud.fit.vubr.cz)
# Date:   2021-04-11

from abc import ABC, abstractmethod
from typing import Callable, Optional, List, Union

from data_structures import DataType, Variable, ArgumentDefinition, InterpreterContext
from exceptions import OperandException, InvalidStringOperationError, StackMissingValueError, InvalidValueError, \
    VariableMissingValueError
from interpret_utils import eprint


class Instruction(ABC):
    def __init__(self, name: str, argument_types: List[Optional[ArgumentDefinition]]):
        self.name = name
        self.arg_t = argument_types

    def check_arguments(self, context: InterpreterContext, arguments: List[Variable]):
        if len(arguments) != len(self.arg_t):
            raise OperandException(context, self, f"Invalid number of arguments for instruction '{self.name}'")
        for i in range(len(arguments)):
            if not self.arg_t[i].is_valid(arguments[i]):
                if arguments[i].type == DataType.UNDEFINED:
                    raise VariableMissingValueError(context, self, arguments[i].name)
                else:
                    raise OperandException(context, self,
                                           f"Invalid value for argument #{i + 1} of instruction '{self.name}'")

    def argument_count(self):
        return len(self.arg_t)

    @abstractmethod
    def process(self, context: InterpreterContext, arguments: List[Variable]):
        raise NotImplementedError()


class BinaryOperandInstruction(Instruction, ABC):
    def __init__(self, name: str, operand_a_type: ArgumentDefinition,
                 operand_b_type: Optional[ArgumentDefinition] = None):
        super().__init__(name, [ArgumentDefinition(None, var_only=True), operand_a_type,
                                operand_a_type if operand_b_type is None else operand_b_type])

    def check_arguments(self, context: InterpreterContext, arguments: List[Variable]):
        super().check_arguments(context, arguments)
        if arguments[1].type != arguments[2].type:
            raise OperandException(context, self, 'Invalid argument types combination.')


class InlineInstruction(Instruction):
    def __init__(self, name: str, argument_types: List[ArgumentDefinition],
                 processor: Union[Callable[[InterpreterContext, List[Variable]], None], Callable[
                     [InterpreterContext, List[Variable], Instruction], None]]):
        super().__init__(name, argument_types)
        self.processor = processor
        import inspect
        self.pass_self = len(inspect.getfullargspec(processor).args) == 3

    def process(self, context: InterpreterContext, arguments: List[Variable]):
        self.check_arguments(context, arguments)
        if self.pass_self:
            self.processor(context, arguments, self)
        else:
            self.processor(context, arguments)


class MoveInstruction(Instruction):
    def __init__(self):
        super().__init__('MOVE', [ArgumentDefinition(None, const_only=False, var_only=True), ArgumentDefinition(None)])

    def process(self, context: InterpreterContext, arguments: List[Variable]):
        self.check_arguments(context, arguments)

        if arguments[1].type == DataType.UNDEFINED:
            raise VariableMissingValueError(context, self, arguments[1].name)

        arguments[0].set_value(arguments[1].value)


class ArithmeticInstruction(BinaryOperandInstruction):
    def __init__(self, name: str):
        super().__init__(name, ArgumentDefinition([DataType.INT, DataType.FLOAT]))

    def process(self, context: InterpreterContext, arguments: List[Variable]):
        self.check_arguments(context, arguments)

        if self.name == 'ADD':
            r = arguments[1].value + arguments[2].value
        elif self.name == 'SUB':
            r = arguments[1].value - arguments[2].value
        elif self.name == 'MUL':
            r = arguments[1].value * arguments[2].value
        elif self.name == 'DIV':  # not having the IDIV instruction here is intended
            try:
                r = arguments[1].value / arguments[2].value
            except ZeroDivisionError:
                raise InvalidValueError(context, self, 'Division by zero')

        else:
            raise Exception('Internal error: invalid arithmetic instruction definition')

        arguments[0].set_value(r)


class ComparisonInstruction(BinaryOperandInstruction):
    def __init__(self, name: str):
        super().__init__(name, ArgumentDefinition([DataType.INT, DataType.FLOAT, DataType.BOOL, DataType.STR]))

    def process(self, context: InterpreterContext, arguments: List[Variable]):
        self.check_arguments(context, arguments)

        if self.name == 'LT':
            r = arguments[1].value < arguments[2].value
        elif self.name == 'GT':
            r = arguments[1].value > arguments[2].value
        else:
            raise Exception('Internal error: invalid arithmetic instruction definition')

        arguments[0].set_value(r)


def _eq_instr_process(context: InterpreterContext, arguments: List[Variable], instruction: Instruction):
    if not (arguments[1].type == arguments[2].type or arguments[1].type == DataType.NIL or
            arguments[2].type == DataType.NIL):
        raise OperandException(context, instruction, 'Invalid argument types combination.')

    arguments[0].set_value(arguments[1].value == arguments[2].value)


class IntToCharInstruction(Instruction):
    def __init__(self):
        super().__init__('INT2CHAR', [ArgumentDefinition(None, var_only=True), ArgumentDefinition(DataType.INT)])

    def process(self, context: InterpreterContext, arguments: List[Variable]):  # TODO: wtf why no check_arguments
        self.check_arguments(context, arguments)

        '''
        if len(arguments) != len(self.arg_t):
            raise OperandException(context, self, f"Invalid number of arguments for instruction '{self.name}'")

        if not self.arg_t[0].is_valid(arguments[0]):
            raise OperandException(context, self,
                                   f"Invalid value for argument #0 of instruction '{self.name}'")
        '''

        try:
            target_char = chr(arguments[1].value)
            arguments[0].set_value(target_char)
        except (TypeError, ValueError) as e:
            raise InvalidStringOperationError(context, self, 'Invalid ordinal value') from e


class ReadInstruction(Instruction):
    def __init__(self):
        super().__init__('READ', [ArgumentDefinition(None, var_only=True),
                                  ArgumentDefinition(DataType.TYPE, const_only=True)])

    def process(self, context: InterpreterContext, arguments: List[Variable]):
        self.check_arguments(context, arguments)
        in_line = context.input_file.readline()
        if in_line == '':
            arguments[0].set_value(None)
            return

        t = arguments[1].value
        try:
            if t == DataType.INT:
                arguments[0].set_value(int(in_line.strip()))
            elif t == DataType.FLOAT:
                arguments[0].set_value(float.fromhex(in_line.strip()))
            elif t == DataType.STR:
                arguments[0].set_value(in_line.rstrip('\r\n'))
            elif t == DataType.BOOL:
                arguments[0].set_value(in_line.strip().lower() == 'true')
            else:
                raise OperandException(context, self, "Invalid type, must be one of 'int', 'string', 'bool' or 'float'")
        except ValueError:
            arguments[0].set_value(None)


def _write_instr_process(context: InterpreterContext, arguments: List[Variable], instruction: Instruction):
    if arguments[0].type == DataType.UNDEFINED:
        raise VariableMissingValueError(context, instruction, arguments[0].name)

    if arguments[0].type == DataType.BOOL:
        print('true' if arguments[0].value else 'false', end='')
    elif arguments[0].type == DataType.NIL:
        return
    elif arguments[0].type == DataType.FLOAT:
        print(arguments[0].value.hex(), end='')
    else:
        print(arguments[0].value, end='')


def _idiv_instr_process(context: InterpreterContext, arguments: List[Variable], instruction: Instruction):
    if arguments[2].value == 0:
        raise InvalidValueError(context, instruction, 'Division by zero')

    arguments[0].set_value(arguments[1].value // arguments[2].value)


class ConcatInstruction(BinaryOperandInstruction):
    def __init__(self):
        super().__init__('CONCAT', ArgumentDefinition(DataType.STR))

    def process(self, context: InterpreterContext, arguments: List[Variable]):
        self.check_arguments(context, arguments)
        arguments[0].set_value(arguments[1].value + arguments[2].value)


class GetCharInstruction(Instruction):
    def __init__(self, ord_mode: bool):
        super().__init__('STRI2INT' if ord_mode else 'GETCHAR',
                         [ArgumentDefinition(None, var_only=True), ArgumentDefinition(DataType.STR),
                          ArgumentDefinition(DataType.INT)])
        self.ord_mode = ord_mode

    def process(self, context: InterpreterContext, arguments: List[Variable]):
        try:
            self.check_arguments(context, arguments)
        except ValueError as e:
            raise InvalidStringOperationError(context, self, 'Invalid operand') from e

        if arguments[2].value < 0 or arguments[2].value >= len(arguments[1].value):
            raise InvalidStringOperationError(context, self, 'Index out of bounds')

        char = arguments[1].value[arguments[2].value]
        arguments[0].set_value(ord(char) if self.ord_mode else char)


class SetCharInstruction(Instruction):
    def __init__(self):
        super().__init__('SETCHAR',
                         [ArgumentDefinition(DataType.STR, var_only=True), ArgumentDefinition(DataType.INT),
                          ArgumentDefinition(DataType.STR)])

    def process(self, context: InterpreterContext, arguments: List[Variable]):
        self.check_arguments(context, arguments)

        if arguments[1].value < 0 or arguments[1].value >= len(arguments[0].value):
            raise InvalidStringOperationError(context, self, 'Index out of bounds')

        if len(arguments[2].value) == 0:
            raise InvalidStringOperationError(context, self, 'Empty source string')

        char = arguments[2].value[0]
        s = arguments[0].value
        s = s[:arguments[1].value] + char + s[(arguments[1].value + 1):]
        arguments[0].value = s


class JumpIfEqInstruction(Instruction):
    def __init__(self, negate: bool):
        super().__init__('JUMPIFNEQ' if negate else 'JUMPIFEQ',
                         [ArgumentDefinition(DataType.LABEL, const_only=True),
                          ArgumentDefinition(None), ArgumentDefinition(None)])
        self.negate = negate

    def process(self, context: InterpreterContext, arguments: List[Variable]):
        self.check_arguments(context, arguments)
        if arguments[1].type == DataType.UNDEFINED:
            raise VariableMissingValueError(context, self, arguments[1].name)
        if arguments[2].type == DataType.UNDEFINED:
            raise VariableMissingValueError(context, self, arguments[2].name)
        if arguments[1].type != arguments[2].type and not (
                arguments[1].type == DataType.NIL or arguments[2].type == DataType.NIL):
            raise OperandException(context, self, 'Types of compared symbols do not match')

        cond = (arguments[1].value == arguments[2].value) != self.negate
        if cond:
            context.jump(arguments[0].value)
        else:
            context.lookup_label(arguments[0].value)


class StackInstructionWrapper(Instruction):
    def __init__(self, parent_instruction: Instruction):
        super().__init__(parent_instruction.name + 'S', [])
        self.parent_instruction = parent_instruction

    def process(self, context: InterpreterContext, arguments: List[Variable]):
        self.check_arguments(context, arguments)

        to_pop = len(self.parent_instruction.arg_t) - 1
        if len(context.variable_stack) < to_pop:
            raise StackMissingValueError(context, self)

        temp_var = Variable('_stack_tmp')
        if to_pop == 2:  # binary parent instructions
            second_arg = context.pop_variable()
            args = [temp_var, context.pop_variable(), second_arg]
        else:  # unary parent instructions
            args = [temp_var, context.pop_variable()]

        self.parent_instruction.process(context, args)
        context.push_variable(temp_var)


class StackJumpInstructionWrapper(Instruction):
    def __init__(self, parent_instruction: Instruction):
        super().__init__(parent_instruction.name + 'S', [ArgumentDefinition(DataType.LABEL, const_only=True)])
        self.parent_instruction = parent_instruction

    def process(self, context: InterpreterContext, arguments: List[Variable]):
        self.check_arguments(context, arguments)

        to_pop = 2
        if len(context.variable_stack) < to_pop:
            raise StackMissingValueError(context, self)

        second_arg = context.pop_variable()
        args = [arguments[0], context.pop_variable(), second_arg]

        self.parent_instruction.process(context, args)
        if context.performing_lookup_jump:
            context.push_variable(args[1])
            context.push_variable(args[2])


AD = ArgumentDefinition
DT = DataType

instruction_defs = [
    MoveInstruction(),
    InlineInstruction('CREATEFRAME', [],
                      lambda c, a: c.make_temp_frame()),
    InlineInstruction('PUSHFRAME', [],
                      lambda c, a: c.push_frame()),
    InlineInstruction('POPFRAME', [],
                      lambda c, a: c.pop_frame()),
    # DEFVAR is handled by the interpret directly
    InlineInstruction('CALL', [AD(DT.LABEL, const_only=True)],
                      lambda c, a: c.call(a[0].value)),
    InlineInstruction('RETURN', [],
                      lambda c, a: c.pop_return()),
    InlineInstruction('PUSHS', [AD(None)],
                      lambda c, a: c.push_variable(a[0])),
    InlineInstruction('POPS', [AD(None, var_only=True)],
                      lambda c, a: a[0].set_value(c.pop_variable().value)),
    InlineInstruction('CLEARS', [],
                      lambda c, a: c.variable_stack.clear()),
    ArithmeticInstruction('ADD'),
    ArithmeticInstruction('SUB'),
    ArithmeticInstruction('MUL'),
    ArithmeticInstruction('DIV'),
    InlineInstruction('IDIV', [AD(None, var_only=True), AD(DT.INT),
                               AD(DT.INT)], _idiv_instr_process),
    ComparisonInstruction('LT'),
    ComparisonInstruction('GT'),
    InlineInstruction('EQ',
                      [AD(None, var_only=True),
                       AD([DT.INT, DT.FLOAT, DT.BOOL, DT.STR, DT.NIL]),
                       AD([DT.INT, DT.FLOAT, DT.BOOL, DT.STR, DT.NIL])],
                      _eq_instr_process),
    InlineInstruction('AND', [AD(None, var_only=True), AD(DT.BOOL),
                              AD(DT.BOOL)],
                      lambda c, a: a[0].set_value(a[1].value and a[2].value)),
    InlineInstruction('OR', [AD(None, var_only=True), AD(DT.BOOL),
                             AD(DT.BOOL)],
                      lambda c, a: a[0].set_value(a[1].value or a[2].value)),
    InlineInstruction('NOT', [AD(None, var_only=True), AD(DT.BOOL)],
                      lambda c, a: a[0].set_value(not a[1].value)),
    IntToCharInstruction(),
    ReadInstruction(),
    InlineInstruction('WRITE', [AD(None)], _write_instr_process),
    ConcatInstruction(),
    InlineInstruction('STRLEN', [AD(None, var_only=True), AD(DT.STR)],
                      lambda c, a: a[0].set_value(len(a[1].value))),
    GetCharInstruction(True),  # STRI2INT
    GetCharInstruction(False),  # GETCHAR,
    SetCharInstruction(),
    InlineInstruction('TYPE', [AD(None, var_only=True), AD(None)],
                      lambda c, a: a[0].set_value(a[1].type.value)),
    InlineInstruction('LABEL', [AD(DT.LABEL, const_only=True)],
                      lambda c, a: c.add_label(a[0].value)),
    InlineInstruction('JUMP', [AD(DT.LABEL, const_only=True)],
                      lambda c, a: c.jump(a[0].value)),
    JumpIfEqInstruction(False),  # JUMPIFEQ
    JumpIfEqInstruction(True),  # JUMPIFNEQ
    InlineInstruction('EXIT', [AD(DT.INT)],
                      lambda c, a: c.terminate(a[0].value)),
    InlineInstruction('DPRINT', [AD(None)],
                      lambda c, a: eprint(a[0].value)),
    InlineInstruction('BREAK', [], lambda c, a: c.dump()),
    InlineInstruction('INT2FLOAT', [AD(None, var_only=True), AD([DT.INT])],
                      lambda c, a: a[0].set_value(float(a[1].value))),
    InlineInstruction('FLOAT2INT', [AD(None, var_only=True), AD([DT.FLOAT])],
                      lambda c, a: a[0].set_value(int(a[1].value))),
    InlineInstruction('DEFVAR', [AD(None, var_only=True)], lambda c, a: None)  # Dummy
]

# Convert list of instructions to a dictionary (lookup table)
instruction_defs = {i.name: i for i in instruction_defs}

# Stack instructions
stack_instructions = ['ADD', 'SUB', 'MUL', 'DIV', 'IDIV', 'LT', 'GT', 'EQ', 'AND', 'OR', 'NOT', 'INT2CHAR',
                      'STRI2INT']
for si in stack_instructions:
    instruction_defs[si + 'S'] = StackInstructionWrapper(instruction_defs[si])

instruction_defs['JUMPIFEQS'] = StackJumpInstructionWrapper(instruction_defs['JUMPIFEQ'])
instruction_defs['JUMPIFNEQS'] = StackJumpInstructionWrapper(instruction_defs['JUMPIFNEQ'])
