<?php
/*
 * IPP project, task 1: parse.php
 * Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)
 * Date:   2021-02-25
 */

ini_set('display_errors', 'stderr');

/**
 * Returned when the processed line is empty (or only contains a comment).
 */
const LINE_EMPTY = 1;
/**
 * Returned when the instruction code is not valid.
 */
const INVALID_OPCODE = 2;
/**
 * Returned when the arguments cannot be parsed.
 */
const INVALID_FORMAT = 3;
/**
 * Returned when the processed line contains the prefix (.IPPcode21).
 */
const CODE_HEADER = 4;

/**
 * The language name to check for in the header and to write to the output XML.
 */
const LANGUAGE_NAME = 'IPPcode21';

/**
 * Instances of this class represent a specific type of input instruction with 0 to 3 arguments.
 * Based on the types of arguments passed to constructor, this class generates a regular expression
 * that can be used to parse and match arguments of the instruction; it also contains a method that extracts
 * information about an argument from an array of matches created by using the previously mentioned regular expression
 * on an input line.
 */
class Rule
{
    public const VAR = 1;
    public const SYMB = 2;
    public const SYMB_BOOL = 3;
    public const SYMB_INT = 4;
    public const SYMB_STRING = 5;
    public const LABEL = 6;
    public const TYPE = 7;

    /**
     * Holds information about the number of match groups in the regex fragment
     * used for the corresponding argument type.
     */
    public const OFFSETS = array(
        Rule::VAR => 2,
        Rule::SYMB => 10,
        Rule::SYMB_BOOL => 4,
        Rule::SYMB_INT => 4,
        Rule::SYMB_STRING => 4,
        Rule::TYPE => 1,
        Rule::LABEL => 1
    );

    private string $opcode;
    private int $arg1;
    private int $arg2;
    private int $arg3;
    private int $argNum;

    private string $regex;

    /**
     * Rule constructor. Initializes a Rule object with the specified argument types.
     * The arg1, arg2 and arg3 parameters specify the types of arguments valid for the instruction.
     * Their values should be one of VAR, SYMB, SYMB_*, LABEL or TYPE constants, or zero if the argument is not used.
     * @param string $opcode Instruction code, case-insensitive.
     * @param int $arg1 Type of the first argument.
     * @param int $arg2 Type of the second argument.
     * @param int $arg3 Type of the third argument.
     */
    public function __construct(string $opcode, int $arg1 = 0, int $arg2 = 0, int $arg3 = 0)
    {
        $this->opcode = $opcode;
        $this->arg1 = $arg1;
        $this->arg2 = $arg2;
        $this->arg3 = $arg3;
        $this->argNum = 0;

        if ($arg1) $this->argNum++;
        if ($arg2) $this->argNum++;
        if ($arg3) $this->argNum++;

        if (($arg2 && !$arg1) || ($arg3 && !$arg2)) {
            throw new InvalidArgumentException("Argument types must be defined sequentially.");
        }

        $this->makeRegex();
    }

    /**
     * Assembles a regex based on the argument types of this Rule instance.
     */
    private function makeRegex()
    {
        // opcode regex would be "[ \\t]*(?i)($this->opcode)(?-i)[ \\t]*"
        $this->regex = '/^[ \t]*' . $this->makeArgRegex($this->arg1)
            . $this->makeArgRegex($this->arg2) . $this->makeArgRegex($this->arg3) . '$/';
    }

    /**
     * Returns a part of the argument-matching regex that matches the specified argument type.
     * @param int $type One of VAR, SYMB, SYMB_*, LABEL or TYPE constants.
     * @return string A regex fragment.
     */
    private function makeArgRegex(int $type): string
    {
        switch ($type) {
            case self::VAR:
                return '(GF|TF|LF)@([a-zA-Z_\-$&%*!?][a-zA-Z0-9_\-$&%*!?]*)[ \t]*';
            case self::SYMB:
                return '(?:(GF|TF|LF)@([a-zA-Z_\-$&%*!?][a-zA-Z0-9_\-$&%*!?]*)|(int)@([+-]?[0-9]+)|(bool)@(true|false)|(nil)@(nil)|(string)@((?:\\\\[0-9]{3}|[^\s\\\\])*))[ \t]*';
            case self::SYMB_BOOL:
                return '(?:(GF|TF|LF)@([a-zA-Z_\-$&%*!?][a-zA-Z0-9_\-$&%*!?]*)|(bool)@(true|false))[ \t]*';
            case self::SYMB_INT:
                return '(?:(GF|TF|LF)@([a-zA-Z_\-$&%*!?][a-zA-Z0-9_\-$&%*!?]*)|(int)@([+-]?[0-9]+))[ \t]*';
            case self::SYMB_STRING:
                return '(?:(GF|TF|LF)@([a-zA-Z_\-$&%*!?][a-zA-Z0-9_\-$&%*!?]*)|(string)@((?:\\\\[0-9]{3}|[^\s\\\\])*))[ \t]*';
            case self::LABEL:
                return '([a-zA-Z_\-$&%*!?][a-zA-Z0-9_\-$&%*!?]*)[ \t]*';
            case self::TYPE:
                return '(int|string|bool)[ \t]*';
        }

        return '';
    }

    /**
     * Extracts the type and value of the argument on the specified position from an array of matches
     * returned by the regex matching function applied on the regex created from this Rule instance.
     * @param array $matches An array of matches.
     * @param int $argNum Position of argument to extract information about. Can be 1, 2 or 3.
     * @return ArgumentValue|null An ArgumentValue object holding the extracted data,
     * or null if the matches array doesn't have the structure expected for this Rule.
     * @noinspection PhpMissingBreakStatementInspection
     */
    public function getArgumentValue(array $matches, int $argNum): ?ArgumentValue
    {
        if ($argNum < 1 || $argNum > $this->argNum) {
            return null;
        }

        $offset = 0;
        switch ($argNum) {
            case 3:
                $offset += self::OFFSETS[$this->arg2];
            case 2:
                $offset += self::OFFSETS[$this->arg1];
        }

        $argType = $argNum == 1 ? $this->arg1 : ($argNum == 2 ? $this->arg2 : $this->arg3);

        switch ($argType) {
            case self::VAR:
                return new ArgumentValue('var', "{$matches[$offset + 1]}@{$matches[$offset + 2]}");
            case self::LABEL:
                return new ArgumentValue('label', $matches[$offset + 1]);
            case self::TYPE:
                return new ArgumentValue('type', $matches[$offset + 1]);
            default:
                return $this->getSymbolValue($matches, $argType, $offset);
        }
    }

    /**
     * A helper function for getArgumentValue() used to extract argument data for the SYMB and SYMB_* argument types.
     * @param array $matches An array of matches.
     * @param int $symbType One of SYMB or SYMB_* constants.
     * @param int $offset The offset in the matches array, based on position of the queried argument.
     * @return ArgumentValue|null An ArgumentValue object holding the extracted data,
     * or null if the matches array doesn't have the structure expected for this Rule.
     */
    private function getSymbolValue(array $matches, int $symbType, int $offset): ?ArgumentValue
    {
        if ($matches[$offset + 1] !== '') {
            // A variable has been matched
            return new ArgumentValue('var', "{$matches[$offset + 1]}@{$matches[$offset + 2]}");
        }

        if ($symbType != self::SYMB) {
            // Bool, string or int has been matched using one of the SYMB_* rules
            return new ArgumentValue($matches[$offset + 3], $matches[$offset + 4]);
        }

        // We must find out which type has been matched
        if ($matches[$offset + 3] === 'int') {
            return new ArgumentValue('int', $matches[$offset + 4]);
        } elseif ($matches[$offset + 5] === 'bool') {
            return new ArgumentValue('bool', $matches[$offset + 6]);
        } elseif ($matches[$offset + 7] === 'nil') {
            return new ArgumentValue('nil', 'nil');
        } elseif ($matches[$offset + 9] === 'string') {
            return new ArgumentValue('string', $matches[$offset + 10]);
        } else {
            fwrite(STDERR, "Internal error: regex matched but correct values couldn't have been extracted.\n");
            return null;
        }
    }

    /**
     * Returns a regular expression (regex pattern) used to match arguments
     * of an instruction denoted by this Rule instance.
     * @return string A regular expression.
     */
    public function getRegex(): string
    {
        return $this->regex;
    }

    /**
     * Returns the instruction code of the instruction denoted by this Rule instance.
     * @return string An instruction code.
     */
    public function getOpcode(): string
    {
        return $this->opcode;
    }

    /**
     * Returns the number of arguments of the instruction denoted by this Rule instance.
     * @return int The number of arguments.
     */
    public function getNumberOfArgs(): int
    {
        return $this->argNum;
    }
}

/**
 * Instances of this class hold two strings, representing a value of a single argument of an input instruction.
 */
class ArgumentValue
{
    private string $type;
    private string $value;

    public function __construct(string $type, string $value)
    {
        $this->type = $type;
        $this->value = $value;
    }

    public function getType(): string
    {
        return $this->type;
    }

    public function getValue(): string
    {
        return $this->value;
    }
}

/**
 * Instances of this class represent an instruction parsed into an opcode and argument values.
 * They are acquired by calling the static method make() that creates a ParsedInstruction instance based
 * on a rule it was matched by and the array of matches of the argument part of the input instruction.
 */
class ParsedInstruction
{
    private string $opcode;
    private array $args;
    private Rule $rule;

    /**
     * Extracts all argument values from the specified array of matches and returns a ParsedInstruction
     * instance representing a loaded and parsed input instruction.
     * @param Rule $rule The rule used to get the specified array of matches.
     * @param array $matches The array of matches.
     * @return int|ParsedInstruction A ParsedInstruction instance,
     * or INVALID_FORMAT if the arguments could not have been extracted.
     */
    public static function make(Rule $rule, array $matches)
    {
        $ret = new ParsedInstruction($rule);
        $ret->args = array();

        for ($i = 0; $i < $rule->getNumberOfArgs(); $i++) {
            $val = $rule->getArgumentValue($matches, $i + 1);

            if ($val == null) {
                $i++;
                fwrite(STDERR, "Cannot parse argument #$i of instruction '{$rule->getOpcode()}'.\n");
                return INVALID_FORMAT;
            }

            array_push($ret->args, $val);
        }

        return $ret;
    }

    private function __construct(Rule $rule)
    {
        $this->opcode = $rule->getOpcode();
        $this->rule = $rule;
    }

    /**
     * Outputs the parsed instruction represented by this instance to the XML writer.
     * @param XMLWriter $writer
     * @param int $order
     */
    public function write(XMLWriter $writer, int $order)
    {
        $writer->startElement('instruction');

        $writer->writeAttribute('order', "$order");
        $writer->writeAttribute('opcode', strtoupper($this->opcode));

        $argCount = count($this->args);
        for ($i = 0; $i < $argCount; $i++) {
            $argNum = $i + 1;

            $writer->startElement("arg$argNum");
            $writer->writeAttribute('type', $this->args[$i]->getType());

            $val = $this->args[$i]->getValue();
            $val = str_replace('&', '&amp;', $val);
            $val = str_replace('<', '&lt;', $val);
            $val = str_replace('>', '&gt;', $val);

            $writer->writeRaw($val);
            $writer->endElement();
        }

        $writer->endElement();
    }

    public function getOpcode(): string
    {
        return $this->opcode;
    }

    /**
     * Returns an ArgumentValue instance describing an argument at the specified position,
     * or null if this instruction has less arguments.
     * @param int $index The index (zero-based) of the argument.
     * @return ArgumentValue|null An ArgumentValue instance filled with type and value of the argument, or null.
     */
    public function getArg(int $index): ?ArgumentValue
    {
        if ($index >= count($this->args)) return null;
        return $this->args[$index];
    }
}

/**
 * Instances of this class represent a single code statistics collecting group.
 * Specific statistics must be allowed using enableFeature(); only the allowed statistics will then be saved
 * into a file by calling save().
 */
class StatsCollector
{
    private string $file;
    private array $features;

    private int $instructions, $comments, $labels, $jumps, $fwJumps, $bckJumps, $badJumps;

    public function __construct(string $file)
    {
        $this->file = $file;
        $this->features = array();
    }

    /**
     * Enables collection of a specific statistic.
     * @param string $feature The statistic to be enabled.
     * Must be one of loc, comments, labels, jumps, fwjumps, backjumps or badjumps.
     */
    public function enableFeature(string $feature)
    {
        array_push($this->features, $feature);
    }

    /**
     * Configures the statistics' values to be saved to the output file.
     * @param int $instructions The number of instructions.
     * @param int $comments The number of comments.
     * @param JumpStatsProcessor $jumps A JumpStatsProcessor instance to collect data about jumps from.
     */
    public function setValues(int $instructions, int $comments, JumpStatsProcessor $jumps)
    {
        $this->instructions = $instructions;
        $this->comments = $comments;
        $this->labels = $jumps->getLabels();
        $this->jumps = $jumps->getJumps();
        $this->fwJumps = $jumps->getForwardJumps();
        $this->bckJumps = $jumps->getBackwardJumps();
        $this->badJumps = $jumps->getBadJumps();
    }

    /**
     * Saves the statistics values (set using setValues) into the file associated with this StatsCollector instance.
     * @return bool True if saving was successful, false when an I/O error occurs.
     */
    public function save(): bool
    {
        $file = fopen($this->file, 'w');
        if (!$file) {
            fwrite(STDERR, "Couldn't open stats file '{$this->file}'.");
            return false;
        }

        foreach ($this->features as $f) {
            switch ($f) {
                case 'loc':
                    fwrite($file, "{$this->instructions}\n");
                    break;
                case 'comments':
                    fwrite($file, "{$this->comments}\n");
                    break;
                case 'labels':
                    fwrite($file, "{$this->labels}\n");
                    break;
                case 'jumps':
                    fwrite($file, "{$this->jumps}\n");
                    break;
                case 'fwjumps':
                    fwrite($file, "{$this->fwJumps}\n");
                    break;
                case 'backjumps':
                    fwrite($file, "{$this->bckJumps}\n");
                    break;
                case 'badjumps':
                    fwrite($file, "{$this->badJumps}\n");
                    break;
            }
        }

        if (!fclose($file)) {
            fwrite(STDERR, "Error closing stats file '{$this->file}'.");
            return false;
        }

        return true;
    }

    public function getTargetFile(): string
    {
        return $this->file;
    }
}

/**
 * Instances of this class can track label definitions and jumps and calculate the number of jumps,
 * forward jumps, backward jumps and bad jumps.
 */
class JumpStatsProcessor
{
    private array $records = array();
    private int $labels = 0, $jumps = 0, $fw, $bck, $bad;

    private bool $calculated = false;

    /**
     * Calculates the numbers of forward, backward and bad jumps.
     * Saves the result into the corresponding instance variables and sets the $calculated flag to true.
     */
    private function calculate()
    {
        // Sort records based on keys, which correspond to lines
        ksort($this->records);
        $this->calculated = true;

        $labels = array();
        $this->fw = $this->bad = $this->bck = 0;

        // Walk through the records array once, collecting labels and evaluating jumps.
        // Consider all jumps to not yet defined labels forward jumps.
        foreach ($this->records as $rec) {
            if ($rec[0] == 'label') {
                array_push($labels, $rec[1]);
            } elseif ($rec[0] == 'jump') {
                if (in_array($rec[1], $labels)) {
                    $this->bck++;
                } else {
                    $this->fw++;
                }
            }
        }

        // Walk through the records array for the second time, now that all labels have been collected.
        // Check whether jump records point to one of the collected labels, if not, decrease the forward jumps
        // counter (in the previous cycle, we didn't know whether the label would later be defined or not)
        // and increase the bad jumps counter.
        foreach ($this->records as $rec) {
            if ($rec[0] == 'jump') {
                if (!in_array($rec[1], $labels)) {
                    $this->fw--;
                    $this->bad++;
                }
            }
        }
    }

    /**
     * Saves information about a label with a specific name having been defined on a specific line.
     * @param int $line The input line the label was defined on.
     * @param string $name The name of the label.
     */
    public function label(int $line, string $name)
    {
        if (in_array(array('label', $name), $this->records)) {
            fwrite(STDERR, "Warning: Label '$name' redefined on line $line, statistics may contain unexpected values.");
            return;
        }

        $this->records[$line] = array('label', $name);
        $this->labels++;
        $this->calculated = false;
    }

    /**
     * Saves information about a jump to a specific label or about a 'return' jump having been defined on a specific line.
     * @param int $line The input line the jump has occurred on.
     * @param string|null $target The name of the target label, or null in case of 'return' jumps.
     */
    public function jump(int $line, ?string $target)
    {
        if ($target == null) {
            $this->records[$line] = array('return', null);
        } else {
            $this->records[$line] = array('jump', $target);
        }

        $this->jumps++;
        $this->calculated = false;
    }

    public function getLabels(): int
    {
        return $this->labels;
    }

    public function getJumps(): int
    {
        return $this->jumps;
    }

    public function getForwardJumps(): int
    {
        if (!$this->calculated) {
            $this->calculate();
        }

        return $this->fw;
    }

    public function getBackwardJumps(): int
    {
        if (!$this->calculated) {
            $this->calculate();
        }

        return $this->bck;
    }

    public function getBadJumps(): int
    {
        if (!$this->calculated) {
            $this->calculate();
        }

        return $this->bad;
    }
}

/**
 * Definitions of rules for all the accepted instructions.
 */
$rules = array(
    "MOVE" => new Rule("move", Rule::VAR, Rule::SYMB),
    "CREATEFRAME" => new Rule("createFrame"),
    "PUSHFRAME" => new Rule("pushFrame"),
    "POPFRAME" => new Rule("popFrame"),
    "DEFVAR" => new Rule("defVar", Rule::VAR),
    "CALL" => new Rule("call", Rule::LABEL),
    "RETURN" => new Rule("return"),
    "PUSHS" => new Rule("pushS", Rule::SYMB),
    "POPS" => new Rule("popS", Rule::VAR),
    "ADD" => new Rule("add", Rule::VAR, Rule::SYMB, Rule::SYMB),
    "SUB" => new Rule("sub", Rule::VAR, Rule::SYMB, Rule::SYMB),
    "MUL" => new Rule("mul", Rule::VAR, Rule::SYMB, Rule::SYMB),
    "IDIV" => new Rule("iDiv", Rule::VAR, Rule::SYMB, Rule::SYMB),
    "LT" => new Rule("lt", Rule::VAR, Rule::SYMB, Rule::SYMB),
    "GT" => new Rule("gt", Rule::VAR, Rule::SYMB, Rule::SYMB),
    "EQ" => new Rule("eq", Rule::VAR, Rule::SYMB, Rule::SYMB),
    "AND" => new Rule("and", Rule::VAR, Rule::SYMB_BOOL, Rule::SYMB_BOOL),
    "OR" => new Rule("or", Rule::VAR, Rule::SYMB_BOOL, Rule::SYMB_BOOL),
    "NOT" => new Rule("not", Rule::VAR, Rule::SYMB_BOOL),
    "INT2CHAR" => new Rule("int2char", Rule::VAR, Rule::SYMB_INT),
    "STRI2INT" => new Rule("stri2int", Rule::VAR, Rule::SYMB_STRING, Rule::SYMB_INT),
    "READ" => new Rule("read", Rule::VAR, Rule::TYPE),
    "WRITE" => new Rule("write", Rule::SYMB),
    "CONCAT" => new Rule("concat", Rule::VAR, Rule::SYMB_STRING, Rule::SYMB_STRING),
    "STRLEN" => new Rule("strLen", Rule::VAR, Rule::SYMB_STRING),
    "GETCHAR" => new Rule("getChar", Rule::VAR, Rule::SYMB_STRING, Rule::SYMB_INT),
    "SETCHAR" => new Rule("setChar", Rule::VAR, Rule::SYMB_INT, Rule::SYMB_STRING),
    "TYPE" => new Rule("type", Rule::VAR, Rule::SYMB),
    "LABEL" => new Rule("label", Rule::LABEL),
    "JUMP" => new Rule("jump", Rule::LABEL),
    "JUMPIFEQ" => new Rule("jumpIfEq", Rule::LABEL, Rule::SYMB, Rule::SYMB),
    "JUMPIFNEQ" => new Rule("jumpIfNeq", Rule::LABEL, Rule::SYMB, Rule::SYMB),
    "EXIT" => new Rule("exit", Rule::SYMB_INT),
    "DPRINT" => new Rule("dPrint", Rule::SYMB),
    "BREAK" => new Rule("break"),
    // Stack instructions
    "CLEARS" => new Rule("clearS"),
    "ADDS" => new Rule("addS"),
    "SUBS" => new Rule("subS"),
    "MULS" => new Rule("mulS"),
    "IDIVS" => new Rule("iDivS"),
    "LTS" => new Rule("ltS"),
    "GTS" => new Rule("gtS"),
    "EQS" => new Rule("eqS"),
    "ANDS" => new Rule("andS"),
    "ORS" => new Rule("orS"),
    "NOTS" => new Rule("notS"),
    "INT2CHARS" => new Rule("int2charS"),
    "STRI2INTS" => new Rule("stri2intS"),
    "JUMPIFEQS" => new Rule("jumpIfEqS", Rule::LABEL),
    "JUMPIFNEQS" => new Rule("jumpIfNeqS", Rule::LABEL)
);

/**
 * Parses a single input line and returns a ParsedInstruction instance or a status code.
 *
 * @param string $line The input line to parse.
 * @param int $comments
 * @return int|ParsedInstruction An ParsedInstruction instance or one of LINE_EMPTY, INVALID_OPCODE, INVALID_FORMAT
 * or CODE_HEADER.
 */
function parseLine(string $line, int &$comments)
{
    global $rules;

    // Strip whitespaces at the beginning and at the end
    $line = trim($line);

    // Remove comments
    if (($commentPos = strpos($line, '#')) !== false) {
        $line = substr($line, 0, $commentPos);
        $comments++;
    }

    // Check for header (trim from both ends to allow whitespaces)
    if (strcasecmp(trim($line), '.' . LANGUAGE_NAME) === 0) {
        return CODE_HEADER;
    }

    // Check for blank line
    if (strlen($line) === 0) {
        return LINE_EMPTY;
    }

    // Replace tabs with spaces
    $line = str_replace("\t", ' ', $line);

    // Get opcode (first word)
    if ($spacePos = strpos($line, ' ')) {
        $opcode = substr($line, 0, $spacePos);
    } else {
        $opcode = $line;
    }

    // Convert to upper case
    $opcode = strtoupper($opcode);

    if (array_key_exists($opcode, $rules)) {
        if ($spacePos) {
            // Parse instruction with arguments
            return parseInstruction($rules[$opcode], ltrim(substr($line, $spacePos)));
        } else {
            // Parse instruction with no arguments
            return parseInstruction($rules[$opcode], '');
        }
    } else {
        fwrite(STDERR, "Invalid instruction code '$opcode'.\n");
        return INVALID_OPCODE;
    }
}

/**
 * Tries to match an input arguments string with the specified Rule and returns a ParsedInstruction instance
 * when the match is successful.
 * @param Rule $rule The Rule instance specifying the instruction to match for.
 * @param string $arguments The arguments part of the input line.
 * @return int|ParsedInstruction A ParsedInstruction object if the matching and parsing of arguments is successful,
 * or INVALID_FORMAT if it's not.
 */
function parseInstruction(Rule $rule, string $arguments)
{
    $res = preg_match($rule->getRegex(), $arguments, $matches);

    if ($res) {
        return ParsedInstruction::make($rule, $matches);
    } else {
        fwrite(STDERR, "Invalid argument format for instruction '{$rule->getOpcode()}'.\n");
        return INVALID_FORMAT;
    }
}

/**
 * Reads lines from input, processes them and invokes the output generation.
 * If the statsCollectors parameter is not null, also invokes generation of the stats files defined by the
 * StatsCollector objects in this array.
 * @param array|null $statsCollectors An array of StatsCollector objects.
 */
function process(?array $statsCollectors = null)
{
    $xml = new XMLWriter();
    $xml->openURI('php://output');
    $xml->startDocument('1.0', 'UTF-8');
    $xml->setIndent(true);

    $hadHeader = false;
    $order = 1;
    $lineNum = 1;

    // Stats
    $comments = 0;
    $instructions = 0;
    $jumps = new JumpStatsProcessor();

    // Read input lines until the end of the input stream
    while ($line = fgets(STDIN)) { // TODO: stdin
        $p = parseLine($line, $comments);

        if ($p === LINE_EMPTY) continue;
        if ($hadHeader && $p === CODE_HEADER) {
            fwrite(STDERR, "Unexpected header at line $lineNum.\n");
            exit(22);
        }

        if (!$hadHeader) {
            if ($p !== CODE_HEADER) {
                fwrite(STDERR, "Unexpected token at line $lineNum, no header found.\n");
                exit(21);
            } else {
                $hadHeader = true;
                $xml->startElement('program');
                $xml->writeAttribute('language', LANGUAGE_NAME);
                continue;
            }
        }

        if ($p === INVALID_OPCODE) {
            fwrite(STDERR, "Error at line $lineNum, terminating.\n");
            exit(22);
        } elseif ($p === INVALID_FORMAT) {
            fwrite(STDERR, "Error at line $lineNum, terminating.\n");
            exit(23);
        } else {
            // Stats
            $instructions++;

            switch ($p->getOpcode()) {
                case 'label':
                    $jumps->label($lineNum, $p->getArg(0)->getValue());
                    break;
                case 'jump':
                case 'jumpIfEq':
                case 'jumpIfNeq':
                case 'jumpIfEqS':
                case 'jumpIfNeqS':
                case 'call':
                    $jumps->jump($lineNum, $p->getArg(0)->getValue());
                    break;
                case 'return':
                    $jumps->jump($lineNum, null);
                    break;
            }

            // Write element
            $p->write($xml, $order++);
        }

        $lineNum++;
    }

    $xml->endElement();
    $xml->flush();

    if ($statsCollectors != null) {
        foreach ($statsCollectors as $collector) {
            $collector->setValues($instructions, $comments, $jumps);
            if (!$collector->save()) {
                exit(12);
            }
        }
    }
}

// Checks for --help in the script arguments. If it is present, outputs help and exits the script.
if (in_array('--help', $argv)) {
    if ($argc > 2) {
        fwrite(STDERR, "--help cannot be used with other parameters.\n");
        exit(10);
    }

    echo "This program reads an IPPcode21 program from the standard input (stdin), checks its lexical and syntactical"
        . " correctness and outputs an XML representation of the input program to the standard output (stdout).\n\n"
        . "Optionally, it can collect statistics about the input program.\nStatistics collection is enabled using"
        . " a --stats=\"output file\" argument. After this argument, additional arguments should follow, enabling"
        . " specific statistic outputs:\n  --comments: outputs the number of comments in the input file\n"
        . "  --loc: outputs the number of instructions in the input file\n"
        . "  --labels: outputs the number of defined labels\n"
        . "  --jumps: outputs the number of all jumping and return instructions\n"
        . "  --fwjumps: outputs the number of forward jumps\n"
        . "  --backjumps: outputs the number of backward jumps\n"
        . "  --badjumps: outputs the number of jumps to undefined labels\n"
        . "Each statistic will be written to a separate line in the output file, in the same order as defined"
        . " in the command line.\n"
        . "Multiple statistics output files with different statistics may be created by using the --stats argument"
        . " repeatedly. For example:\n"
        . "  parse.php --stats=stat1 --comments --loc --stats=stat2 --fwjumps\n"
        . "would create two files: 'stat1' with two lines with the number of comments and number on instruction on them,"
        . "and 'stat2' with one line with the number of forward jumps on it.\n\n"
        . "Possible error return codes:\n  - 10: invalid command line arguments\n  - 12: error opening an output file\n"
        . "  - 21: missing header of the input file\n  - 22: invalid instruction code in the input file\n"
        . "  - 23: other lexical or syntactic error (e.g. invalid instruction argument)\n";
    exit(0);
}

// Parses the script arguments. If stats collection arguments are present, creates corresponding StatsCollector objects
// that encapsulate the requested collection options.
$collectors = array();
if ($argc > 1) {
    $opts = array_slice($argv, 1);
    $currentProc = null;
    $validStatArgs = array('comments', 'loc', 'labels', 'jumps', 'fwjumps', 'backjumps', 'badjumps');

    foreach ($opts as $opt) {
        if (strpos($opt, '--stats=') === 0) {
            $file = substr($opt, strpos($opt, '=') + 1);

            if (!$file || strlen($file) == 0) {
                // No output file in this argument
                exit(10);
            }

            if (array_key_exists($file, $collectors)) {
                // Output file already specified
                exit(12);
            }

            if ($currentProc != null) {
                $collectors[$currentProc->getTargetFile()] = $currentProc;
            }

            $currentProc = new StatsCollector($file);
        } else {
            if ($currentProc == null) {
                // No output file specified before
                exit(10);
            }

            if (strpos($opt, '--') != 0) {
                // Not a valid argument
                exit(10);
            }

            $opt = substr($opt, 2);
            if (!in_array($opt, $validStatArgs)) {
                // Not a valid stats option
                exit(10);
            }

            $currentProc->enableFeature($opt);
        }
    }

    if ($currentProc != null) {
        $collectors[$currentProc->getTargetFile()] = $currentProc;
    }
}

// Invokes the input processing
process(count($collectors) == 0 ? null : $collectors);
