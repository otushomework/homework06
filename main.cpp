#include <list>
#include <iostream>
#include <fstream>
#include <functional>

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
    LogWriter()
    {
        m_logFile.open ("/tmp/bulk.txt");
    }

    ~LogWriter()
    {
        m_logFile.close();
    }

    void write(std::list<std::string> commands)
    {
        m_logFile << "bulk:";
        for (auto command : commands)
            m_logFile << command << " ";
        m_logFile << std::endl;
    }

private:
    std::ofstream m_logFile;
};

//$ bulk < bulk1.txt
int main(int, char *[])
{
    Parser parser(3);

    Executor executor;
    parser.subscribe(std::bind(&Executor::execute, &executor, std::placeholders::_1));

    LogWriter logWriter;
    parser.subscribe(std::bind(&LogWriter::write, &logWriter, std::placeholders::_1));

    parser.exec();

    return 0;
}
