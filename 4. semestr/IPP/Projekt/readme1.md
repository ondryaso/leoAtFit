Documentation of Project 1 Implementation for IPP 2020/2021 \
Name and surname: Ondřej Ondryáš \
Login: xondry02

The submitted `parse.php` script supports the base functionality as well as collecting code statistics (as defined in
the `STATP` extension).

It has been implemented in a way that allows extending its parsing capabilities without having to modify it extensively.
This has been achieved by applying an object model to the process, splitting it into classes that encapsulate different
aspects of the process. (Albeit, the author chose this approach mainly to familiarise himself with PHP and its
implementation of the OOP paradigm.)

## Parsing approach

The input instructions are parsed using a combination of program logic and regular expression matching. The script reads
the standard input line by line. Each line is preprocessed by trimming leading and trailing whitespaces, removing
comments and replacing tabs with spaces. (This is done in the `parseLine` function.) Then, the first _word_ (sequence of
characters from the beginning to the first occurrence of a space, or to the end of the line if no space is present) is
extracted, converted to upper case and looked up in a pre-defined collection of _rules_ (described below). If a
corresponding _rule_ is found, meaning that the extracted word equals to one of the supported instruction codes, the
remainder of the input line (which should contain the arguments of the instruction) is processed by the _rule_'s
associated regular expression (in the `parseInstruction` function). If it matches, the collection of resulting match
groups values is further processed to extract the argument types and values. The result of this process is an instance
of the `ParsedInstruction` class, which can then write its data to the output XML.

### Rules

The accepted language is defined using rules. A rule can be viewed as a tuple of instruction code and zero to three
argument types. Based on the argument types, a regular expression can be assembled that is used to parse arguments from
a source instruction line. In code, rules are instances of the `Rule` class and argument types are represented using
named integer constants `VAR` (variable), `SYMB` (variable or any constant), `SYMB_[BOOL|INT|STRING]` (variable or
constant of a specific type), `LABEL` and `TYPE` (type name) defined in the `Rule` class. This allows a basic type
checking of constants on the syntax analysis level. Supported literal types could easily be extended by adding a new
type constant and corresponding regular expression.

The regular expressions are created automatically during construction. They contain several matching groups that
effectively split the input argument line into several parts, which are then used to determine the type and the value of
the argument. The `getArgumentValue()` method of the `Rule` class is used to extract values of arguments from the array
of matches, as it involves calculating the location of the required data in the array and, in case of symbols, finding
which match group contains the actual data.

The program uses a global array of rules. Adding a new supported instruction would thus only be a matter of adding a
new `Rule` instance into this array.

## Statistics

The statistics collection functionality uses two classes: \
`StatsCollector` represents a single collection group – one output file requested using a `--stats` command line
argument. After the parsing has finished, all `StatsCollector`
objects must be filled with collected values of statistics; then, it handles saving the output file.

The statistics calculation is encapsulated in `JumpStatsProcessor`. A single instance of this class is created when
starting the parsing process. Its public interface consists of the `label` and `jump` tracking methods, and of a getter
for all supported statistics. Internally, it walks through its array of tracked instructions and collects data about
jumps. This is only done once when one of the jump statistics values is requested.