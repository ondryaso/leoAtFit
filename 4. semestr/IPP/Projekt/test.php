<?php
/*
 * IPP project, task 2, part 2: test.php
 * Author: Ondřej Ondryáš (xondry02@stud.fit.vutbr.cz)
 * Date:   2021-04-20
 */

ini_set('display_errors', 'stderr');

const MODE_BOTH = 1;
const MODE_PARSER = 2;
const MODE_INTERPRETER = 3;

const PHP_INT_PATH = '/usr/local/bin/php7.4';
const PYTHON_INT_PATH = '/usr/local/bin/python3.8';
const JAVA_PATH = '/usr/bin/java';
const DIFF_PATH = '/usr/bin/diff';

/**
 * Makes a filesystem path out by joining the specified path segments using a OS-agnostic directory separator.
 * Source: https://stackoverflow.com/a/50515077
 * @param string ...$parts Path segments to join.
 * @return string A joined path.
 */
function paths_join(string ...$parts): string
{
    $parts = array_map('trim', $parts);
    $path = [];

    foreach ($parts as $part) {
        if ($part !== '') {
            $path[] = $part;
        }
    }

    $path = implode(DIRECTORY_SEPARATOR, $path);

    return preg_replace(
        '#' . preg_quote(DIRECTORY_SEPARATOR) . '{2,}#',
        DIRECTORY_SEPARATOR,
        $path
    );
}

/**
 * Container for the script's configuration (command line arguments).
 */
class Config
{
    private string $parserPath;
    private string $interpreterPath;
    private string $jexamJar;
    private string $jexamConfig;
    private int $mode;
    private bool $captureOut;

    /**
     * Config constructor.
     * @param string $parserPath Path to the tested parser.php script.
     * @param string $interpreterPath Path to the tested interpret.py script.
     * @param string $jexamJar Path to the JExamXML JAR file used for XML comparison.
     * @param string $jexamConfig Path to a config file for the JExamXML tool.
     * @param bool $captureOut Determines whether outputs of the tested components
     * will be captured and present in the generated report.
     * @param int $mode Determines the testing mode (one of MODE_PARSER, MODE_INTERPRETER, MODE_BOTH).
     */
    public function __construct(string $parserPath, string $interpreterPath, string $jexamJar,
                                string $jexamConfig, bool $captureOut, int $mode)
    {
        $this->parserPath = PHP_INT_PATH . ' ' . $parserPath;
        $this->interpreterPath = PYTHON_INT_PATH . ' ' . $interpreterPath;
        $this->jexamJar = $jexamJar;
        $this->jexamConfig = $jexamConfig;
        $this->mode = $mode;
        $this->captureOut = $captureOut;
    }

    /**
     * @return string Path to the tested parser.php script, prepended with the PHP_INT_PATH constant value.
     */
    public function getParserPath(): string
    {
        return $this->parserPath;
    }

    /**
     * @return string Path to the tested interpret.py script, prepended with the PYTHON_INT_PATH constant value.
     */
    public function getInterpreterPath(): string
    {
        return $this->interpreterPath;
    }

    /**
     * @return string Path to the JExamXML JAR file used for XML comparison.
     */
    public function getJexamJar(): string
    {
        return $this->jexamJar;
    }

    /**
     * @return string Path to a config file for the JExamXML tool.
     */
    public function getJexamConfig(): string
    {
        return $this->jexamConfig;
    }

    /**
     * @return int The testing mode (one of MODE_PARSER, MODE_INTERPRETER, MODE_BOTH).
     */
    public function getMode(): int
    {
        return $this->mode;
    }

    /**
     * @return bool A bool signalizing whether outputs of the tested components
     * will be captured and present in the generated report.
     */
    public function shouldCaptureOut(): bool
    {
        return $this->captureOut;
    }
}

/**
 * Instances of this class represent a single test and the current state of execution of that test. The
 * test may be run using the run() method; the mode of execution is determined by the corresponding value
 * in the passed Config object. After the test has been run, its outputs relevant to the current mode of execution
 * are accessible using getter methods of this class.
 */
class TestSuite
{
    // These constants represent different possible outcomes of test executions.
    // Their values also determine sorting of the test records in the output report.
    public const NOT_RUN = 0;
    public const PARSER_WRONG_RC = 1;
    public const INTERPRETER_WRONG_RC = 2;
    public const PARSER_WRONG_OUTPUT = 3;
    public const INTERPRETER_WRONG_OUTPUT = 4;
    public const SUCCESS = 5;

    private string $sourceFile, $programInputFile, $expectedOutputFile, $expectedRcFile, $dir;
    private string $name;
    private int $expectedRc;
    private int $parserRc = -1, $interpreterRc = -1;
    private string $parserErr = 'not captured', $interpreterErr = 'not captured';
    private string $parserOut = 'not captured', $interpreterOut = 'not captured';
    private string $diffOut = 'not captured';
    private string $errMsg = '';
    private float $time = 0.0;
    private Config $config;
    private int $result = self::NOT_RUN;

    /**
     * TestSuite constructor.
     * @param string $testDir Path of the directory where the test files are.
     * @param string $testName Name of the test (without any extension like .src or .rc).
     * @param Config $config Configuration container.
     */
    public function __construct(string $testDir, string $testName, Config $config)
    {
        $this->sourceFile = paths_join($testDir, $testName . '.src');
        $this->programInputFile = paths_join($testDir, $testName . '.in');
        $this->expectedOutputFile = paths_join($testDir, $testName . '.out');
        $this->expectedRcFile = paths_join($testDir, $testName . '.rc');
        $this->dir = $testDir;
        $this->name = $testName;
        $this->config = $config;
    }

    /**
     * @return int Result of execution (one of [PARSER|INTERPRETER]_WRONG_[RC|OUTPUT], SUCCESS or NOT_RUN).
     */
    public function getResult(): int
    {
        return $this->result;
    }

    /**
     * @return string Name of this test.
     */
    public function getName(): string
    {
        return $this->name;
    }

    /**
     * @return string Message of the last error generated during the last test suite run.
     * An empty string if no error has occurred.
     */
    public function getErrorMessage(): string
    {
        return $this->errMsg;
    }

    /**
     * @return string Data captured from stderr during the parser execution,
     * or 'not captured' if the parser has not been run.
     */
    public function getParserErrorOutput(): string
    {
        return $this->parserErr;
    }

    /**
     * @return string Data captured from stderr during the interpreter execution,
     * or 'not captured' if the parser has not been run.
     */
    public function getInterpretErrorOutput(): string
    {
        return $this->interpreterErr;
    }

    /**
     * @return string Data captured from the diff tool output when comparing
     * the interpreter output to the expected one in runDiffComparison(),
     * or 'not captured' if diff has not been run.
     */
    public function getDiffOutput(): string
    {
        return $this->diffOut;
    }

    /**
     * @return int Expected return code, as defined in the corresponding test file.
     */
    public function getExpectedReturnCode(): int
    {
        return $this->expectedRc;
    }

    /**
     * @return int Exit code returned by the parser after its execution,
     * or -1 if the parser has not been run.
     */
    public function getParserReturnCode(): int
    {
        return $this->parserRc;
    }

    /**
     * @return int Exit code returned by the interpreter after its execution,
     * or -1 if the interpreter has not been run.
     */
    public function getInterpreterReturnCode(): int
    {
        return $this->interpreterRc;
    }

    /**
     * @return string Path to this test's source file (.src).
     */
    public function getSourceFile(): string
    {
        return $this->sourceFile;
    }

    /**
     * @return float Total execution time in milliseconds, or 0.0 if this test suite has not been run yet.
     */
    public function getTime(): float
    {
        return round(floatval($this->time) / 1000000.0, 1);
    }

    /**
     * @return string Data captured from stdout during the parser execution,
     * or 'not captured' if the parser has not been run.
     */
    public function getParserOutput(): string
    {
        return $this->parserOut;
    }

    /**
     * @return string Data captured from stdout during the interpreter execution,
     * or 'not captured' if the parser has not been run.
     */
    public function getInterpreterOutput(): string
    {
        return $this->interpreterOut;
    }

    /**
     * @return string Path to this test's expected output file (.out).
     */
    public function getExpectedOutputFile(): string
    {
        return $this->expectedOutputFile;
    }

    /**
     * @return string The directory from which this test suite was loaded.
     */
    public function getTestFilesDirectory(): string
    {
        return $this->dir;
    }

    /**
     * Executes this test suite in the mode specified in the current Config object.
     * Checks existence of the input files (creates missing files if necessary), executes the parser,
     * the interpreter or both; if they finish successfully, executes the corresponding difference tool
     * and checks whether their output matches the expected output.
     * Sets the corresponding fields of this TestSuite which may be retrieved using the getter methods.
     * If an exception occurs, sets the result code of this TestSuite to NOT_RUN. However, this code is never
     * returned as the return value of this method.
     * Measures the execution time.
     * @return int Result of execution (one of [PARSER|INTERPRETER]_WRONG_[RC|OUTPUT], SUCCESS).
     * @throws TestException Thrown when an error occurs while attempting to execute an external program.
     */
    public function run(): int
    {
        $this->processInputFiles();

        $interpreterOutFile = '';

        $startTime = hrtime(true);
        switch ($this->config->getMode()) {
            case MODE_PARSER:
                $parserOutFile = $this->runParser();

                if ($this->parserRc != $this->expectedRc) {
                    $this->time = hrtime(true) - $startTime;
                    unlink($parserOutFile);
                    return ($this->result = self::PARSER_WRONG_RC);
                }

                if ($this->expectedRc == 0) {
                    try {
                        return ($this->result = $this->runParserXmlComparison($parserOutFile));
                    } catch (TestException $e) {
                        $this->result = self::NOT_RUN;
                        $this->errMsg = $e->getMessage();
                        throw $e;
                    } finally {
                        $this->time = hrtime(true) - $startTime;
                        unlink($parserOutFile);
                    }
                }

                $this->time = hrtime(true) - $startTime;
                unlink($parserOutFile);
                return ($this->result = self::SUCCESS);
            case MODE_BOTH:
                $parserOutFile = $this->runParser();

                // In the 'both' mode, parser can return a non-zero exit code, in that case,
                // it must be the expected one. If it returns a zero, the interpreter is always run,
                // regardless of the expected rc. Parser output is never checked here.
                if ($this->parserRc != 0 && $this->parserRc != $this->expectedRc) {
                    $this->time = hrtime(true) - $startTime;
                    unlink($parserOutFile);
                    return ($this->result = self::PARSER_WRONG_RC);
                }

                try {
                    $interpreterOutFile = $this->runInterpreter($parserOutFile);
                } catch (TestException $e) {
                    $this->result = self::NOT_RUN;
                    $this->errMsg = $e->getMessage();
                    throw $e;
                } finally {
                    unlink($parserOutFile);
                }

                break;
            case MODE_INTERPRETER:
                try {
                    $interpreterOutFile = $this->runInterpreter($this->sourceFile);
                } catch (TestException $e) {
                    $this->result = self::NOT_RUN;
                    $this->errMsg = $e->getMessage();
                    throw $e;
                }

                break;
        }

        // We're not in the parser-only mode; Check interpreter RC and its output.
        if ($this->interpreterRc != $this->expectedRc) {
            $this->time = hrtime(true) - $startTime;
            unlink($interpreterOutFile);
            return ($this->result = self::INTERPRETER_WRONG_RC);
        }

        if ($this->expectedRc == 0) {
            try {
                return ($this->result = $this->runDiffComparison($interpreterOutFile));
            } catch (TestException $e) {
                $this->result = self::NOT_RUN;
                $this->errMsg = $e->getMessage();
                throw $e;
            } finally {
                $this->time = hrtime(true) - $startTime;
                unlink($interpreterOutFile);
            }
        }

        $this->time = hrtime(true) - $startTime;
        unlink($interpreterOutFile);
        return ($this->result = self::SUCCESS);
    }

    /**
     * Checks the input files, creates missing files and loads the expected return code value. Throws on error.
     * @throws TestException Thrown when the source file doesn't exist, when a file cannot be created
     * or when the .rc file doesn't contain a valid integer value.
     */
    private function processInputFiles()
    {
        if (!file_exists($this->sourceFile)) {
            throw new TestException($this, 'test source file does not exist.', 11);
        }

        if (!file_exists($this->programInputFile)) {
            if (file_put_contents($this->programInputFile, '') === false) {
                throw new TestException($this, 'cannot create empty .in file.', 12);
            }
        }

        if (!file_exists($this->expectedOutputFile)) {
            if (file_put_contents($this->expectedOutputFile, '') === false) {
                throw new TestException($this, 'cannot create empty .out file.', 12);
            }
        }

        if (!file_exists($this->expectedRcFile)) {
            if (file_put_contents($this->expectedRcFile, "0\n") === false) {
                throw new TestException($this, 'cannot create the default .rc file.', 12);
            }
        }

        if (($rcContent = file_get_contents($this->expectedRcFile)) === false) {
            throw new TestException($this, 'cannot read the .rc file.', 11);
        }

        if (!is_numeric(trim($rcContent))) {
            throw new TestException($this, 'invalid return code value in the .rc file.', 11);
        }

        $this->expectedRc = intval(trim($rcContent));
    }

    /**
     * Runs the parser. Contents of the current .src file are piped into its stdin. Its stdout is stored
     * in a temporary file (path to this file is returned). Its stderr output is captured and stored in memory.
     * If capturing output is enabled in Config, reads the contents of the output temporary file and stores it,
     * so that it can be retrieved using getParserOutput().
     * @return string Path to a temporary file with the parser's output.
     * @throws TestException Thrown when the temporary file cannot be created or when the parser cannot be executed.
     */
    private function runParser(): string
    {
        $tempFile = tempnam(sys_get_temp_dir(), 'ipp');
        if ($tempFile === false) {
            throw new TestException($this, "couldn't create temporary output file.", 12);
        }

        $cwd = getcwd();
        $descriptorSpec = array(
            0 => array('file', $this->sourceFile, 'r'),  // stdin
            1 => array('file', $tempFile, 'w'),  // stdout
            2 => array('pipe', 'w')   // stderr
        );

        $proc = proc_open($this->config->getParserPath(), $descriptorSpec, $pipes, $cwd);
        if ($proc === false) {
            unlink($tempFile);
            throw new TestException($this, "couldn't run parser.", 99);
        }

        $this->parserErr = stream_get_contents($pipes[2]);  // Possible error here is ignored.
        $this->parserRc = proc_close($proc);

        if ($this->config->shouldCaptureOut()) {
            $this->parserOut = file_get_contents($tempFile);
        }

        return $tempFile;
    }

    /**
     * Runs the interpreter. The $inputFile argument is used as the value of its --source command-line parameter.
     * Its stdout is stored in a temporary file (path to this file is returned).
     * Its stderr output is captured and stored in memory.
     * If capturing output is enabled in Config, reads the contents of the output temporary file and stores it,
     * so that it can be retrieved using getInterpreterOutput().
     * @param string $inputFile Path to the file to pass as the source file.
     * @return string Path to a temporary file with the parser's output.
     * @throws TestException Thrown when the temporary file cannot be created or when the interpreter cannot be executed.
     */
    private function runInterpreter(string $inputFile): string
    {
        $tempFile = tempnam(sys_get_temp_dir(), 'ipp');
        if ($tempFile === false) {
            throw new TestException($this, "couldn't create a temporary output file.", 12);
        }

        $cwd = getcwd();
        $descriptorSpec = array(
            // 0 => array('file', $inputFile, 'r'),  // stdin
            1 => array('file', $tempFile, 'w'),  // stdout
            2 => array('pipe', 'w')   // stderr
        );

        $proc = proc_open($this->config->getInterpreterPath() . " --source=\"{$inputFile}\" --input=\"{$this->programInputFile}\"",
            $descriptorSpec, $pipes, $cwd);
        if ($proc === false) {
            unlink($tempFile);
            throw new TestException($this, "couldn't run interpreter.", 99);
        }

        $this->interpreterErr = stream_get_contents($pipes[2]);  // Possible error here is ignored.
        $this->interpreterRc = proc_close($proc);

        if ($this->config->shouldCaptureOut()) {
            $this->interpreterOut = file_get_contents($tempFile);
        }

        return $tempFile;
    }

    /**
     * Runs the JExamXML utility to compare parser output with the expected output.
     * Attempts to read the generated difference file (if it signalises that the files differ) and stores it,
     * so that it can be retrieved using getDiffOutput()
     * (doesn't throw if saving or reading the difference file fails).
     * @param string $parserOutputFile Path to the generated output file to compare.
     * @return int One of SUCCESS or PARSER_WRONG_OUTPUT.
     * @throws TestException Thrown when the tool cannot be run or when it exits with a code other than 0 or 1.
     */
    private function runParserXmlComparison(string $parserOutputFile): int
    {
        // Make a temporary file for the diff
        $tempFile = tempnam(sys_get_temp_dir(), 'ipp');

        // If it fails, it's not really a blocking problem, so let's just go on without reading the diff
        if ($tempFile !== false) {
            $command = JAVA_PATH . " -jar {$this->config->getJexamJar()} {$this->expectedOutputFile} " .
                "{$parserOutputFile} {$tempFile} {$this->config->getJexamConfig()}";
        } else {
            $command = JAVA_PATH . " -jar {$this->config->getJexamJar()} {$this->expectedOutputFile} " .
                "{$parserOutputFile} -O:{$this->config->getJexamConfig()}";
        }

        $execResult = exec($command, $output, $resCode);
        if ($execResult === false) {
            @unlink($tempFile); // This would only issue a warning if temp file didn't exist, suppress it with @
            throw new TestException($this, "couldn't run the XML compare tool.", 99);
        }

        if ($resCode === 0) {
            $this->diffOut = '';
            @unlink($tempFile);
            return self::SUCCESS;
        } else if ($resCode === 1) {
            if ($tempFile !== false) {
                $this->diffOut = file_get_contents($tempFile);
                unlink($tempFile);
            }

            return self::PARSER_WRONG_OUTPUT;
        } else {
            throw new TestException($this, "unexpected error occurred when running the XML compare tool ({$command}).", 99);
        }
    }

    /**
     * Runs the GNU diff utility to compare interpreter output with the expected output.
     * Stores its output, so that it can be retrieved using getDiffOutput().
     * @param string $interpreterOutputFile Path to the generated output file to compare.
     * @return int One of SUCCESS or INTERPRETER_WRONG_OUTPUT.
     * @throws TestException Thrown when the tool cannot be run or when it exits with a code other than 0 or 1.
     */
    private function runDiffComparison(string $interpreterOutputFile): int
    {
        $command = DIFF_PATH . " {$this->expectedOutputFile} {$interpreterOutputFile}";
        $execResult = exec($command, $out, $resCode);
        if ($execResult === false) {
            throw new TestException($this, "couldn't run the diff tool.", 99);
        }

        if ($resCode === 0) {
            return self::SUCCESS;
        } else if ($resCode === 1) {
            $this->diffOut = implode("\n", $out);
            return self::INTERPRETER_WRONG_OUTPUT;
        } else {
            throw new TestException($this, "unexpected error occurred when running the diff tool.", 99);
        }
    }
}

/**
 * TestException is thrown when an error occurs while running a test suite.
 * A TestException instance contains a reference to the failed test and a code identifying the error,
 * which is meant to be used as the exit code of the script.
 */
class TestException extends Exception
{
    private TestSuite $test;

    public function __construct(TestSuite $test, string $message, int $code, Throwable $previous = null)
    {
        parent::__construct("Could not run test {$test->getName()}: " . $message, $code, $previous);
        $this->test = $test;
    }

    public function getTest(): TestSuite
    {
        return $this->test;
    }
}

/**
 * ReportAssembler instance handles loading the output template and populating it with
 * results of finished tests. It implements a basic templating system with placeholders
 * and subsections.
 */
class ReportAssembler
{
    private Config $config;
    private string $template, $invalidRcTemplate, $invalidOutTemplate, $successfulTemplate, $errorsTemplate;

    /**
     * Compares two test suites based on their results, by subtracting their result codes.
     * (See the constants defined in TestSuite.) When the result codes are equal, returns the result
     * of strcmp() comparison of the tests' names.
     * This is used to put the erroneous tests at the beginning of the result list.
     * @param TestSuite $a The first test suite to compare.
     * @param TestSuite $b The second test suite to compare.
     * @return int An integer < 0 if a is less than b; > 0 if a is greater than b, and 0 if they are equal.
     */
    public static function compareTests(TestSuite $a, TestSuite $b): int
    {
        $x = $b->getResult() - $a->getResult();
        if ($x == 0) {
            return strcmp($a->getName(), $b->getName());
        }
        return $x;
    }

    /**
     * ReportAssembler constructor. Loads the specified template HTML file.
     * @param string $templateFile Path to the template file.
     * @param Config $config Configuration container.
     * @throws Exception A general exception is thrown if the contents of the template file cannot be read.
     */
    public function __construct(string $templateFile, Config $config)
    {
        $this->config = $config;
        $this->loadTemplate($templateFile);
    }

    /**
     * Creates an HTML report about the specified test suites.
     * @param array $tests Array of run tests.
     * @param int $totalTime Total running time in milliseconds.
     * @return string Generated HTML report.
     */
    public function makeReport(array $tests, int $totalTime): string
    {
        // Sort the input array using the compareTests static function
        usort($tests, array('ReportAssembler', 'compareTests'));
        $suc = 0; // Number of successful tests
        $unexpRc = 0; // Number of tests with unexpected RC
        $unexpOut = 0; // Number of tests with unexpected output
        $notRun = 0; // Number of tests not run

        $invalidRcTests = [];
        $invalidOutTests = [];
        $sucTests = [];
        $notRunTests = [];
        $dirs = [];

        // Sort (categorise) the test suites based on their result
        foreach ($tests as $test) {
            $res = $test->getResult();
            if ($res == TestSuite::SUCCESS) {
                $suc++;
                array_push($sucTests, $test);
            } elseif ($res == TestSuite::PARSER_WRONG_OUTPUT || $res == TestSuite::INTERPRETER_WRONG_OUTPUT) {
                $unexpOut++;
                array_push($invalidOutTests, $test);
            } elseif ($res == TestSuite::PARSER_WRONG_RC || $res == TestSuite::INTERPRETER_WRONG_RC) {
                $unexpRc++;
                array_push($invalidRcTests, $test);
            } elseif ($res == TestSuite::NOT_RUN) {
                $notRun++;
                array_push($notRunTests, $test);
            }

            if (!in_array($test->getTestFilesDirectory(), $dirs)) {
                array_push($dirs, $test->getTestFilesDirectory());
            }
        }

        // Sanity check (this could happen if some of the test suites ended with an unknown return code)
        $failed = $unexpRc + $unexpOut;
        $total = $suc + $failed;
        if (($total + $notRun) != count($tests)) {
            fwrite(STDERR, "Warning: test count mismatch.\n");
        }

        // Calculate percentages of successful and failed tests
        if ($total > 0) {
            $sucPc = (int)ceil((floatval($suc) / $total) * 100);
            $failedPc = (int)floor((floatval($failed) / $total) * 100);
        } else {
            $sucPc = 0;
            $failedPc = 0;
        }

        // Replace placeholders in the template with calculated values
        $out = str_replace('%total_tests%', $total, $this->template);
        $out = str_replace('%successful_tests%', $suc, $out);
        $out = str_replace('%st_perc%', $sucPc, $out);
        $out = str_replace('%failed_tests%', $failed, $out);
        $out = str_replace('%ft_perc%', $failedPc, $out);
        $out = str_replace('%unexpected_rc%', $unexpRc, $out);
        $out = str_replace('%unexpected_out%', $unexpOut, $out);

        // Create result details lists and populate corresponding placeholders
        $invalidRcOverview = $this->makeFailedTestsOverview($invalidRcTests, true);
        $invalidOutOverview = $this->makeFailedTestsOverview($invalidOutTests, false);
        $sucOverview = $this->makeSuccessfulTestsOverview($sucTests);

        $out = str_replace('%invalid_rc%', $invalidRcOverview, $out);
        $out = str_replace('%invalid_output%', $invalidOutOverview, $out);
        $out = str_replace('%successful%', $sucOverview, $out);

        $out = str_replace('%datetime%', date('Y-m-d, G:i'), $out);
        $out = str_replace('%total_time%', $totalTime, $out);

        // Add quotes to arguments with spaces and populate the command line placeholder
        global $argv;
        $argDecorator = function (string $a) {
            if (strpos($a, ' ') === false) {
                return $a;
            } else {
                return "\"{$a}\"";
            }
        };
        $decoratedArgs = array_map($argDecorator, $argv);
        $out = str_replace('%cmd_line%', join(' ', $decoratedArgs), $out);

        // Populate the tests mode placeholder
        $modes = [MODE_INTERPRETER => 'interpreter only', MODE_PARSER => 'parser only',
            MODE_BOTH => 'both parser and interpreter'];
        $out = str_replace('%mode%', $modes[$this->config->getMode()], $out);

        if (count($dirs) == 0) {
            $out = str_replace('%dirs%', "<li>no tests found</li>", $out);
        } else {
            // Populate the directory list with paths encapsulated in <li>
            $dirsToHtml = function ($dir) {
                $rp = realpath($dir);
                if ($rp != $dir) {
                    return '<li>' . $dir . '&nbsp;&nbsp;(' . rp . ')</li>';
                } else {
                    return '<li>' . $dir . '</li>';
                }
            };
            $out = str_replace('%dirs%', join("\n", array_map($dirsToHtml, $dirs)), $out);
        }

        // If we had erroneous test runs, populate that info
        if ($notRun > 0) {
            $testsToHtml = function (TestSuite $test) {
                return '<li>' . $test->getTestFilesDirectory() . ': ' . $test->getErrorMessage() . '</li>';
            };

            $templ = str_replace('%error_tests%', join("\n", array_map($testsToHtml, $notRunTests)), $this->errorsTemplate);
            $out = str_replace('%errors%', $templ, $out);
        } else {
            $out = str_replace('%errors%', '', $out);
        }

        return $out;
    }

    /**
     * Creates content for the 'failed tests' part of the report.
     * @param array $tests Array of the failed tests.
     * @param bool $failedOnRc Boolean signalising whether the tests failed because of invalid return code
     * or because of invalid output. This is used to choose the proper template.
     * @return string Generated HTML.
     */
    private function makeFailedTestsOverview(array $tests, bool $failedOnRc): string
    {
        $ret = '';
        /* @var $test TestSuite */  // Type hint
        foreach ($tests as $test) {
            $testResult = $test->getResult();
            if ($testResult == TestSuite::INTERPRETER_WRONG_RC || $testResult == TestSuite::INTERPRETER_WRONG_OUTPUT) {
                $phase = 'interpreter';
                $rc = $test->getInterpreterReturnCode();
                $eo = htmlspecialchars($test->getInterpretErrorOutput());
                $o = htmlspecialchars($test->getInterpreterOutput());
            } else {
                $phase = 'parser';
                $rc = $test->getParserReturnCode();
                $eo = htmlspecialchars($test->getParserErrorOutput());
                $o = htmlspecialchars($test->getParserOutput());
            }

            $out = str_replace('%test_name%', $test->getName(),
                $failedOnRc ? $this->invalidRcTemplate : $this->invalidOutTemplate);
            $out = str_replace('%phase_name%', $phase, $out);
            $out = str_replace('%expected_rc%', $test->getExpectedReturnCode(), $out);
            $out = str_replace('%actual_rc%', $rc, $out);
            $out = str_replace('%test_src%', $test->getSourceFile(), $out);
            $out = str_replace('%test_time%', $test->getTime(), $out);
            $out = str_replace('%error_output%', $eo, $out);
            $out = str_replace('%output%', $o, $out);

            if ($this->config->shouldCaptureOut()) {
                $expOut = file_get_contents($test->getExpectedOutputFile());
                if ($expOut === false) {
                    $out = str_replace('%expected_output%', 'error reading', $out);
                } else {
                    $out = str_replace('%expected_output%', htmlspecialchars($expOut), $out);
                }
            } else {
                $out = str_replace('%expected_output%', 'not captured', $out);
            }

            $out = str_replace('%diff%', htmlspecialchars($test->getDiffOutput()), $out);
            $ret .= $out . "\n";
        }

        return $ret;
    }

    /**
     * Creates content for the 'successful tests' part of the report.
     * @param array $tests Array of the successful tests.
     * @return string Generated HTML.
     */
    private function makeSuccessfulTestsOverview(array $tests): string
    {
        $ret = '';
        /* @var $test TestSuite */  // Type hint
        foreach ($tests as $test) {

            if ($this->config->getMode() == MODE_PARSER) {
                $o = htmlspecialchars($test->getParserOutput());
            } else {
                $o = htmlspecialchars($test->getInterpreterOutput());
            }

            $out = str_replace('%test_name%', $test->getName(), $this->successfulTemplate);
            $out = str_replace('%expected_rc%', $test->getExpectedReturnCode(), $out);
            $out = str_replace('%test_src%', $test->getSourceFile(), $out);
            $out = str_replace('%test_time%', $test->getTime(), $out);
            $out = str_replace('%output%', $o, $out);

            $ret .= $out . "\n";
        }

        return $ret;
    }

    /**
     * Loads the contents of the specified file with an HTML template, extracts template subsections
     * for all the possible test results and stores them in the corresponding instance fields.
     * @param string $file File to load the template from.
     * @throws Exception Thrown when reading the contents of the file fails. Leads to exit code 10.
     */
    private function loadTemplate(string $file)
    {
        $contents = file_get_contents($file);
        if ($contents === false) {
            throw new Exception('Error loading output template file.', 10);
        }

        $this->invalidRcTemplate = $this->extractTemplateSection($contents, 'invalid_rc');
        $this->invalidOutTemplate = $this->extractTemplateSection($contents, 'invalid_output');
        $this->successfulTemplate = $this->extractTemplateSection($contents, 'successful');
        $this->errorsTemplate = $this->extractTemplateSection($contents, 'errors');
        $this->template = $contents;
    }

    /**
     * Finds a template subsection defined by %{$sectionName}_start% and %{$sectionName}_end%, extracts
     * its contents and replaces the whole block in the original template with a %{$sectionName}% placeholder.
     * @param string $template Template to extract the subsection from.
     * @param string $sectionName Subsection identifier.
     * @return string Contents of the subsection.
     */
    private function extractTemplateSection(string &$template, string $sectionName): string
    {
        $start = '%' . $sectionName . '_start%';
        $end = '%' . $sectionName . '_end%';
        $si = strpos($template, $start);
        $se = strpos($template, $end);

        if ($si === false || $se === false) {
            return false;
        }

        $o = $si + strlen($start);
        $sec = substr($template, $o, $se - $o);
        $template = substr($template, 0, $si) . '%' . $sectionName . '%' . substr($template, $se + strlen($end));

        return $sec;
    }
}

$options = getopt('', ['help', 'directory:', 'recursive', 'parse-script:', 'int-script:',
    'parse-only', 'int-only', 'jexamxml:', 'jexamcfg:', 'capture-output', 'testlist:', 'match:', 'status', 'outfile:']);

if (!$options && count($argv) > 1) {  // Running with no arguments is not allowed
    fwrite(STDERR, "Invalid arguments. Run with the '--help' argument to see correct usage.\n");
    exit(10);
}

// Handle the help argument
if (array_key_exists('help', $options)) {
    if (count($options) > 1) {
        fwrite(STDERR, "--help cannot be used with other arguments.\n");
        exit(10);
    }

    echo "This script executes IPPcode21 parser and interpreter tests and generates an HTML report, " .
        "which is printed to the standard output. The tests may either only test the parser, or the interpreter, " .
        "or they can test both components (default behaviour). The following arguments can be used:\n" .
        "  --help: prints this help. Cannot be used with any other argument.\n" .
        "  --directory=[path]: sets the directory to load the tests from. Default value is the current working directory.\n" .
        "  --recursive: enables loading tests from subdirectories recursively.\n" .
        "  --parse-script=[path]: sets path to the 'parse.php' parser script. Cannot be used together with --int-only. " .
        "Default value: './parse.php'.\n" .
        "  --int-script=[path]: sets path to the 'interpret.py' interpreter script. Cannot be used together with --parse-only. " .
        "Default value: './interpret.py'.\n" .
        "  --parse-only: runs tests in parser-only mode. The execution XML output is compared " .
        "to the test's expected output using the JExamXML tool. Cannot be used together with --int-only or --int-script.\n" .
        "  --int-only: runs tests in interpreter-only mode. The execution output is compared to the test's expected " .
        "output using diff. Cannot be used together with --parse-only or --parse-script.\n" .
        "  --jexamxml=[path]: sets path to the JExamXML tool JAR file. Default value: '/pub/courses/ipp/jexamxml/jexamxml.jar'.\n" .
        "  --jexamcfg=[path]: sets path to the JExamXML tool options file. Default value: '/pub/courses/ipp/jexamxml/options'.\n\n" .
        "JExamXML JAR and config files only have to exist when running the script in parser-only mode.\n\n" .
        "FILES extension options:\n" .
        "  --testlist=[path]: loads a list of tests (one path to directory or .src file per line) to run from the specified path. " .
        "Cannot be used together with --directory.\n" .
        "  --match=[regular expression]: only runs tests with names that match the specified regular expression.\n\n" .
        "Custom options:\n" .
        "  --capture-output: captures output of the executed components and includes it in the report.\n" .
        "  --status: prints information about each test run to stderr.\n" .
        "  --outfile=[path]: when used, the report is saved to a file instead of being put to stdout.\n";
    exit(0);
}

// Check the mode specification arguments
if (array_key_exists('parse-only', $options)
    && (array_key_exists('int-only', $options) || array_key_exists('int-script', $options))) {
    fwrite(STDERR, "--parse-only cannot be used together with --int-only or --int-script.\n");
    exit(10);
}

if (array_key_exists('int-only', $options)
    && (array_key_exists('parse-only', $options) || array_key_exists('parse-script', $options))) {
    fwrite(STDERR, "--int-only cannot be used together with --parse-only or --parse-script.\n");
    exit(10);
}

if (array_key_exists('directory', $options) && array_key_exists('testlist', $options)) {
    fwrite(STDERR, "--directory cannot be used together with --testlist.\n");
    exit(10);
}

// Extract argument values, fill in default values for missing ones
$testsDir = $options['directory'] ?? '.';
$recursive = array_key_exists('recursive', $options);
$parseScript = $options['parse-script'] ?? 'parse.php';
$interpreterScript = $options['int-script'] ?? 'interpret.py';
$jexamJar = $options['jexamxml'] ?? '/pub/courses/ipp/jexamxml/jexamxml.jar';
$jexamConfig = $options['jexamcfg'] ?? '/pub/courses/ipp/jexamxml/options';
$mode = array_key_exists('parse-only', $options) ? MODE_PARSER :
    (array_key_exists('int-only', $options) ? MODE_INTERPRETER : MODE_BOTH);
$captureOut = array_key_exists('capture-output', $options);
$testList = $options['testlist'] ?? null;
$outFile = $options['outfile'] ?? null;
$showStatus = array_key_exists('status', $options);
$matchRegex = $options['match'] ?? null;

// Check whether the required files exist
if (!is_dir($testsDir)) {
    fwrite(STDERR, "Tests source directory '{$testsDir}' doesn't exist.\n");
    exit(11);
}

if ($mode == MODE_PARSER || $mode == MODE_BOTH) {
    if (!file_exists($parseScript)) {
        fwrite(STDERR, "Parser script '{$parseScript}' doesn't exist.\n");
        exit(41);
    }

    if ($mode == MODE_PARSER) {
        if (!file_exists($jexamJar)) {
            fwrite(STDERR, "JExamXML executable '{$jexamJar}' doesn't exist.\n");
            exit(41);
        }

        if (!file_exists($jexamConfig)) {
            fwrite(STDERR, "JExamXML configuration file '{$jexamConfig}' doesn't exist.\n");
            exit(41);
        }
    }
}

if ($mode == MODE_INTERPRETER || $mode == MODE_BOTH) {
    if (!file_exists($interpreterScript)) {
        fwrite(STDERR, "Interpreter script '{$interpreterScript}' doesn't exist.\n");
        exit(41);
    }
}

// Load the regular expression, if it is provided, and perform a dummy match to check it (and cache it automatically)
if ($matchRegex != null) {
    // The 'at' sign suppresses PHP warnings
    if (@preg_match($matchRegex, null) === false) {
        fwrite(STDERR, "Invalid regular expression provided.\n");
        exit(11);
    }
}

// Assemble a Config object holding the loaded options
$conf = new Config($parseScript, $interpreterScript, $jexamJar, $jexamConfig, $captureOut, $mode);

// Lad the output template file called 'template.html'.
// This attempts to load the file from the directory in which the parse.php script is located.
// If it fails, it tries to load it from the working directory. If that fails, the script ends.
$scriptDir = dirname(__FILE__);
$templateFile = paths_join($scriptDir, 'template.html');

if (!file_exists($templateFile)) {
    if (!file_exists('template.html')) {
        fwrite(STDERR, "Cannot find template file.\n");
        exit(42);
    } else {
        $templateFile = 'template.html';
    }
}

try {
    // The constructor of this class throws an Exception when the template cannot be read
    $asm = new ReportAssembler($templateFile, $conf);
} catch (Exception $e) {
    fwrite(STDERR, $e->getMessage() . "\n");
    exit($e->getCode());
}

// Prepares an iterator over the test files.
// This is done based on $testList having a value; and the value of $recursive.
if ($testList == null) {
    // If we're not using a test list, either make a recursive directory iterator, or a non-recursive one
    $filesIterator = $recursive ? new RecursiveIteratorIterator(new RecursiveDirectoryIterator($testsDir))
        : new DirectoryIterator($testsDir);
} else {
    // Otherwise create an appendable iterator, load the test list file...

    $filesIterator = new AppendIterator();
    $testListFile = fopen($testList, 'r');
    if ($testListFile === false) {
        fwrite(STDERR, "Error opening the test list.\n");
        exit(41);
    }

    $files = [];
    // and check the lines one by one. Paths that aren't directories and don't end with '.src' are IGNORED!
    while (($line = fgets($testListFile)) !== false) {
        $line = trim($line);

        if (is_dir($line)) {
            // The line contains a directory, add its contents to the appendable iterator
            $it = $recursive ? new RecursiveIteratorIterator(new RecursiveDirectoryIterator($line))
                : new DirectoryIterator($line);
            $filesIterator->append($it);
        } else if (substr_compare(trim($line), '.src', -4) !== 0) {  // Checks whether the path ends with '.src'
            fwrite(STDERR, "Ignoring file '{$line}' in the test list.\n");
            continue;
        } else if (file_exists($line)) {
            // If it's a single file, add it to the auxiliary array, which will later be appended to the iterator all at once
            array_push($files, new SplFileInfo($line));
        } else {
            fwrite(STDERR, "File or directory '{$line}' specified in the test list doesn't exist.\n");
        }
    }

    fclose($testListFile);
    $filesIterator->append(new ArrayIterator($files));
}

$tests = []; // An array of completed tests
$time = hrtime(true); // Start measuring total run time
$actualFiles = [];
$exitCode = 0; // Exit code to returned. This is only overwritten once, when the first TestException is caught.
$counter = 0;
$total = 0;

// First pass: go through the iterator and filter out actual valid (and regex-matching, if need be) source files
foreach ($filesIterator as $file) {
    if ($file->isDir()) {
        continue;
    }

    $path = $file->getPathname();
    if (substr_compare($path, '.src', -4) !== 0) {  // Checks whether the path ends with '.src'
        continue;
    }

    if ($matchRegex != null) {
        if (!preg_match($matchRegex, $file->getBasename('.src'))) {
            continue;
        }
    }

    array_push($actualFiles, $file->getRealPath());
    $total++;
}

// Second pass: create test suites and run them
foreach ($actualFiles as $f) {
    // Pushing the existing SplFileInfo to the $actualFiles array in the first step should have been fine
    // but on merlin, it behaved incorrectly for some unknown reason
    $file = new SplFileInfo($f);
    $test = new TestSuite($file->getPath(), $file->getBasename('.src'), $conf);

    try {
        if ($showStatus) {
            $t = round(((hrtime(true) - $time) / 1000000.0), 1);
            $counter++;
            fwrite(STDERR, "[{$t}] Running {$counter}/{$total}: {$test->getName()}\n");
        }

        $test->run();
    } catch (TestException $e) {
        fwrite(STDERR, "An error occurred while running test {$e->getTest()->getName()}: {$e->getMessage()}\n");
        if ($exitCode == 0) {
            $exitCode = $e->getCode();
        }
    }

    array_push($tests, $test);
}

try {
    // Generates the report.
    $output = $asm->makeReport($tests, (int)ceil((hrtime(true) - $time) / 1000000.0));

    if ($outFile == null) {
        fwrite(STDOUT, $output);
        fwrite(STDOUT, "\n");
        fflush(STDOUT);
    } else {
        $o = fopen($outFile, 'w');
        if ($o === false) {
            fwrite(STDERR, "Cannot open the output file for writing.\n");
            exit(42);
        }

        fwrite($o, $output);
        fclose($o);
    }
} catch (Exception $e) {
    fwrite(STDERR, $e->getMessage() . "\n");
    if ($exitCode == 0) {
        $exitCode = $e->getCode();
    }
}

exit($exitCode);
