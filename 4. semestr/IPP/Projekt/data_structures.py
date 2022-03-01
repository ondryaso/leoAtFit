# IPP project, task 2, part 1: interpret.py
# File:   data_structures.py
# Author: Ondřej Ondryáš (xondry02@stud.fit.vubr.cz)
# Date:   2021-04-11

import codecs
import re
from enum import Enum
from typing import Union, Optional, TextIO, List, Dict

from exceptions import InvalidTypeError, LexicalError, RedefinedVariableError, UndefinedVariableError, \
    ArgumentDefinitionError, UndefinedFrameError, CallStackEmptyError, RedefinedLabelError, StackMissingValueError, \
    InvalidValueError, VariableMissingValueError
from interpret_utils import eprint


class DataType(Enum):
    """Represents a type that a Variable object can have."""
    UNDEFINED = ''  # Undefined variable
    NIL = 'nil'  # Special 'empty' variable but defined
    INT = 'int'  # Integer variable
    STR = 'string'  # String variable
    BOOL = 'bool'  # Boolean variable
    FLOAT = 'float'  # Float variable

    LABEL = 'label'  # Variable encapsulating a label name
    TYPE = 'type'  # Variable (constant) representing a data type

    @classmethod
    def get(cls, name: str):
        """Gets a DataType enum value based on its name, compared case-insensitively"""

        name = name.strip()
        for a in cls:
            if a.value.lower() == name:
                return a

        return DataType.UNDEFINED


class Variable:
    """Represents a named or unnamed data container in the executed program."""

    VarType = Union[int, str, float, bool, DataType, None]

    def __init__(self, name: Optional[str] = None):
        """Creates a variable object. The name argument is optional – unnamed Variable objects represent constants."""
        self.name: str = name
        self.type: DataType = DataType.UNDEFINED
        self.value: Variable.VarType = None

    def set_value(self, value: VarType):
        """Sets the value and the type (inferred from the value) of this Variable instance.

        :raises ValueError: Raised when the value is not one of a supported type."""
        self.value = value

        # bool must be checked before int, because isinstance(5 == 6, int) == True
        if isinstance(value, bool):
            self.type = DataType.BOOL
        elif isinstance(value, str):
            self.type = DataType.STR
        elif isinstance(value, int):
            self.type = DataType.INT
        elif isinstance(value, float):
            self.type = DataType.FLOAT
        elif isinstance(value, DataType):
            self.type = DataType.TYPE
        elif value is None:
            self.type = DataType.NIL
        else:
            raise ValueError('Internal error: invalid variable type')

    def set_value_parse(self, type_name: str, value: str):
        """Parses the specified string value based on the specified type name and sets the value
        and the type of this Variable instance.

        :raises InvalidTypeError: Raised when the type_name argument does not contain a valid type name."""
        var_type = DataType.get(type_name)
        if var_type == DataType.UNDEFINED:
            raise InvalidTypeError()

        if var_type == DataType.LABEL:
            self.type = DataType.LABEL
            self.value = value
            return

        self.set_value(LiteralParser.parse(var_type, value))

    def is_const(self):
        """Returns true if this Variable instance represents a constant."""
        return self.name is None


class LiteralParser:
    """Provides helper methods for parsing literals from the program source."""

    str_regex = re.compile(r'(\\[0-9]{3}(?:\\[0-9]{3})*)')

    @staticmethod
    def parse_int(value: str) -> int:
        try:
            return int(value)
        except ValueError as e:
            raise LexicalError(value, 'int') from e

    @staticmethod
    def convert_escape_seqs(value: re.Match) -> str:
        seq_values = bytes(map(lambda x: int(x), value.group().split('\\')[1:]))
        return codecs.decode(seq_values, 'utf-8')

    @classmethod
    def parse_string(cls, value: str) -> str:
        if value is None:
            return ''

        try:
            return cls.str_regex.sub(LiteralParser.convert_escape_seqs, value)
        except Exception as e:
            raise LexicalError(value, 'string') from e

    @staticmethod
    def parse_bool(value: str) -> bool:
        # Only accept lowercase variants
        if value == 'true':
            return True
        elif value == 'false':
            return False
        else:
            raise LexicalError(value, 'bool')

    @staticmethod
    def parse_float(value: str) -> float:
        try:
            return float.fromhex(value)
        except ValueError as e:
            raise LexicalError(value, 'float') from e

    @staticmethod
    def parse_type(value: str) -> DataType:
        t = DataType.get(value)
        if t == DataType.UNDEFINED or t == DataType.NIL or t == DataType.LABEL or t == DataType.TYPE:
            raise LexicalError(value, 'type')

        return t

    @staticmethod
    def parse(var_type: DataType, value: str) -> Variable.VarType:
        if var_type == DataType.UNDEFINED:
            return None
        if var_type == DataType.NIL:
            if value == 'nil':
                return None
            else:
                raise LexicalError(value, 'nil')

        return {
            DataType.INT: LiteralParser.parse_int,
            DataType.STR: LiteralParser.parse_string,
            DataType.BOOL: LiteralParser.parse_bool,
            DataType.FLOAT: LiteralParser.parse_float,
            DataType.TYPE: LiteralParser.parse_type
        }[var_type](value)


class Frame:
    """Represents a frame of variables of the executed program.
    Provides interface for defining and retrieving variables based on their name."""

    def __init__(self):
        self.vars: dict[str, Variable] = {}

    def has_var(self, var_name: str) -> bool:
        return var_name in self.vars

    def def_var(self, var_name: str) -> Variable:
        if self.has_var(var_name):
            raise RedefinedVariableError(None, None, var_name)

        var = Variable(var_name)
        self.vars[var_name] = var
        return var

    def get_var(self, var_name: str) -> Variable:
        if not self.has_var(var_name):
            raise UndefinedVariableError(None, None, var_name)

        return self.vars[var_name]

    def dump(self):
        if len(self.vars) == 0:
            eprint('The frame is empty.')

        for name, var in self.vars.items():
            eprint(f"Variable '{name}', type {var.type.name}: '{var.value}'")


class ArgumentDefinition:
    """ArgumentDefinition objects are used to define what kinds of arguments an Instruction expects."""

    def __init__(self, arg_type: Optional[Union[DataType, List[DataType]]],
                 const_only: bool = False, var_only: bool = False):
        """Constructs an ArgumentDefinition object. Different constraints may be specified.

        :param arg_type: Specifies a constraint for the argument type. May be either None, which means that
        any type except for LABEL and TYPE but including UNDEFINED is accepted;
        or a single data type or a list of data types.

        :param const_only: When true, only unnamed Variable objects that represent constants are accepted.
        :param var_only: When true, only named Variable objects that represent program variables are accepted.
        """
        self.arg_type = arg_type
        self.catch_all = arg_type is None
        self.const_only = const_only
        self.var_only = var_only

        # Integrity check
        if var_only and const_only:
            raise ArgumentDefinitionError()
        if isinstance(arg_type, list) and (
                DataType.LABEL in arg_type or DataType.TYPE in arg_type or DataType.UNDEFINED in arg_type):
            raise ArgumentDefinitionError()
        if (arg_type == DataType.LABEL or arg_type == DataType.TYPE or arg_type == DataType.NIL) and not const_only:
            raise ArgumentDefinitionError()
        if arg_type == DataType.UNDEFINED:
            raise ArgumentDefinitionError()
        if arg_type is None:
            self.arg_type = [DataType.INT, DataType.STR, DataType.FLOAT, DataType.BOOL, DataType.NIL]

    def is_valid(self, var: Variable):
        """Checks whether the specified Variable object can be accepted as an argument defined
        by this ArgumentDefinition instance."""

        return ((not self.const_only or var.is_const())  # const_only implies var.is_const()
                and (not self.var_only or not var.is_const())  # var_only implies !var.is_const()
                and (var.type == self.arg_type
                     or (var.type == DataType.UNDEFINED and self.catch_all)
                     or (isinstance(self.arg_type, list) and var.type in self.arg_type)))


class InterpreterContext:
    """Represents the current state of program execution."""

    def __init__(self, input_file: TextIO):
        self.global_frame: Frame = Frame()
        self.local_frame: Optional[Frame] = None  # Kept in sync with current top of frame_stack
        self.temp_frame: Optional[Frame] = None
        self.frame_stack: List[Frame] = []
        self.variable_stack: List[Variable] = []
        self.call_stack: List[int] = []
        self.program_counter: int = 0
        self.input_file: TextIO = input_file
        self.labels: Dict[str, int] = {}  # Label cache – pairs of label and program counter value
        self.exit_code: int = -1  # Interpreted program exit code – execution ends when set to a value different to -1
        self.current_jump_start_pc: int = 0  # Program counter of the jump instruction that started the current lookup
        self.current_jump_target: Optional[str] = None  # When set to a string value, label lookup is performed
        # Set to true if program counter should be set back to current_jump_start_pc value when label is found
        self.performing_lookup_jump: bool = False

    def make_temp_frame(self) -> Frame:
        self.temp_frame = Frame()
        return self.temp_frame

    def push_frame(self):
        if self.temp_frame is None:
            raise UndefinedFrameError(self, None, 'TF')

        self.frame_stack.append(self.temp_frame)
        self.local_frame = self.temp_frame
        self.temp_frame = None

    def pop_frame(self):
        if self.local_frame is None:
            raise UndefinedFrameError(self, None, 'LF')

        self.temp_frame = self.frame_stack.pop()
        if len(self.frame_stack) > 0:
            self.local_frame = self.frame_stack[-1]
        else:
            self.local_frame = None

    def push_call(self):
        self.call_stack.append(self.program_counter)

    def pop_return(self) -> int:
        if len(self.call_stack) == 0:
            raise CallStackEmptyError(self, None)

        self.program_counter = self.call_stack.pop()
        return self.program_counter

    def push_variable(self, var: Variable):
        if var.type == DataType.UNDEFINED or var.type == DataType.LABEL or var.type == DataType.TYPE:
            raise VariableMissingValueError(self, None, var.name)

        self.variable_stack.append(var)

    def pop_variable(self) -> Variable:
        if len(self.variable_stack) == 0:
            raise StackMissingValueError(self, None)

        return self.variable_stack.pop()

    def add_label(self, label: str):
        if label in self.labels and self.labels[label] != self.program_counter:
            raise RedefinedLabelError(self, None, label)
        self.labels[label] = self.program_counter

        if self.current_jump_target == label:
            self.current_jump_target = None

    def jump(self, label: str):
        if label not in self.labels:
            self.current_jump_start_pc = self.program_counter
            self.current_jump_target = label
        else:
            self.program_counter = self.labels[label]

    def call(self, label: str):
        self.push_call()
        self.jump(label)

    def lookup_label(self, label: str):
        if label in self.labels:
            return

        self.current_jump_target = label
        self.current_jump_start_pc = self.program_counter
        self.performing_lookup_jump = True

    def terminate(self, exit_code: int):
        if exit_code < 0 or exit_code > 49:
            raise InvalidValueError(self, None, 'Invalid exit code value')
        self.exit_code = exit_code

    def dump(self):
        eprint('-- Global frame GF --')
        self.global_frame.dump()
        eprint('\n-- Frames stack --')
        i = 0
        for loc_fr in self.frame_stack:
            if loc_fr == self.local_frame:
                eprint(f"-- #{i} (current LF)")
            else:
                eprint(f"-- #{i}")
            loc_fr.dump()
            i = i + 1
        eprint('\n-- Temporary frame TF --')
        if self.temp_frame is None:
            eprint('The frame is undefined.')
        else:
            self.temp_frame.dump()
        eprint(f"\nProgram counter: {self.program_counter}")
        if self.current_jump_target is not None:
            eprint(f"Current jump started at PC value: {self.current_jump_start_pc}")
            eprint(f"Current jump target: {self.current_jump_target}")
        eprint('Call stack:\n-- Bottom --')
        for line in self.call_stack:
            eprint(line)
        eprint('-- Top --')
        eprint('\nVariable stack\n-- Bottom --:')
        for var in self.variable_stack:
            eprint(f"{var.type.name}: '{var.value}'")
        eprint('-- Top --')
        eprint('\nDefined labels:')
        for label, line in self.labels.items():
            eprint(f"{label} at line {line}")
