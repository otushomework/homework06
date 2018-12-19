#include <list>
#include <iostream>
#include <fstream>
#include <functional>
#include <stdlib.h>
#include <ctime>
#include <unistd.h>
#include <stdio.h>

class Parser
{
    enum class ParsingState
    {
        TopLevel = 0,
        InBlock = 1
    };

public:
    Parser (int bulkSize)
        : m_bulkSize(bulkSize) { }

    void exec()
    {
        ParsingState state = ParsingState::TopLevel;
        std::list<std::string> commands;
        int depthCounter = 0;

        for(std::string line; std::getline(std::cin, line);)
        {
            switch (state)
            {
            case ParsingState::TopLevel:
            {
                if (line != "{")
                {
                    commands.push_back(line);
                    if (commands.size() == m_bulkSize)
                        publish(commands);
                    break;
                }
                else
                {
                    depthCounter++;
                    publish(commands);
                    state = ParsingState::InBlock;
                    break;
                }
            }
            case ParsingState::InBlock:
            {
                if (line != "}")
                {
                    if (line == "{")
                        depthCounter++;
                    else
                        commands.push_back(line);
                }
                else
                {
                    depthCounter--;
                    if (depthCounter == 0)
                    {
                        publish(commands);
                        state = ParsingState::TopLevel;
                    }
                }
                break;
            }
            default:
                break;
            }
        }

        if (state == ParsingState::TopLevel)
            publish(commands);
    }

    void subscribe(const std::function<void(std::list<std::string>)>& callback)
    {
        m_subscribers.push_back(callback);
    }

    void publish(std::list<std::string> &commands)
    {
        if (commands.size() == 0)
            return;

        for (const auto& subscriber : m_subscribers)
        {
            subscriber(commands);
        }
        commands.clear();
    }

private:
    std::list<std::function<void(std::list<std::string>)> > m_subscribers;
    int m_bulkSize;
};

class Executor
{
public:
    void execute(std::list<std::string> commands)
    {
        std::cout << "bulk:";
        for (auto command : commands)
            std::cout << command << " ";
        std::cout << std::endl;
    }
};

class LogWriter
{
public:
    LogWriter() { }

    ~LogWriter() { }

    void write(std::list<std::string> commands)
    {
        std::ofstream logFile;

        std::time_t result = std::time(nullptr);
        char buff[FILENAME_MAX];
        getcwd(buff, FILENAME_MAX );
        std::string current_working_dir(buff);
        logFile.open (std::string(current_working_dir + "/bulk" + std::to_string(result) + ".log"));
        std::cout << std::string(current_working_dir + "/bulk" + std::to_string(result) + ".log") << std::endl;
        logFile << "bulk:";
        for (auto command : commands)
            logFile << command << " ";
        logFile << std::endl;

        logFile.close();
    }
};

//$ bulk < bulk1.txt
int main(int argc, const char *argv[])
{
    int bulkCount = 5;
    if (argc == 2)
    {
        char *p;
        bulkCount = std::strtol(argv[1], &p, 10);
    }

    Parser parser(bulkCount);

    Executor executor;
    parser.subscribe(std::bind(&Executor::execute, &executor, std::placeholders::_1));

    LogWriter logWriter;
    parser.subscribe(std::bind(&LogWriter::write, &logWriter, std::placeholders::_1));

    parser.exec();

    return 0;
}
