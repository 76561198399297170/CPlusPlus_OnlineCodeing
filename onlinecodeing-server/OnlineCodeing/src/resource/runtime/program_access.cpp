#include "runtime/program_access.h"

ProgramAccess::ProgramStatus ProgramAccess::runProgram(const ProgramAccess::ProgramLimits& limits, const std::string& code, const std::vector<std::vector<std::string>>& test_case_inputs, const std::vector<std::vector<std::string>>& test_case_outputs)
{
    std::unique_ptr<PooledCode> pooled_code = CodePool::getInstance()->getExecutor();
    if (!pooled_code)
    {
        std::cerr << "[warn]ProgramAccess::runProgram: get connection from pool failed." << std::endl;
        return { "retry", "get connection " };
    }
    CodeExecutor* executor = pooled_code->get();

    if (limits.memory_limit_mb == -1)
    {
        std::cerr << "[warn]ProgramAccess::runProgram: memory_limit_mb is -1, use 256mb." << std::endl;
        executor->setMemoryLimit(256);
    }
    else
    {
        executor->setMemoryLimit(limits.memory_limit_mb);
    }

    if (limits.time_limit_ms == -1)
    {
        std::cerr << "[warn]ProgramAccess::runProgram: time_limit_ms is -1, use 3000ms." << std::endl;
        executor->setTimeLimit(3000);
    }
    else
    {
        executor->setTimeLimit(limits.time_limit_ms);
    }

    std::string result = executor->evaluateCode(limits.lang, code, test_case_inputs, test_case_outputs);
    std::string error_msg;
    ProgramAccess::ProgramStatus status;
    status.status = codeResultToString(codeStringFindCodeResult(result, error_msg));
    status.error_message = error_msg;
    status.run_time_ms = executor->getRunTimeMs();
    status.code_size_bytes = executor->getCodeSizeBytes();
    status.test_outputs = executor->getTestOutputs();
    return status;
}
