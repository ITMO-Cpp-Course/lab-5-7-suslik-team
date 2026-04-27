#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

// Временный класс
class Document
{
  public:
    unsigned int id;
    std::string name;
    std::vector<std::string> text;

    Document(unsigned int id_, const std::string& name_, const std::vector<std::string>& text_)
        : id(id_), name(name_), text(text_)
    {
    }
};

class DocumentBuilder
{
  public:
    static Document buildDocument(unsigned int id, const std::string& name, const std::string& words)
    {
        return Document(id, name, splitWords(words));
    }
    static std::vector<std::string> splitWords(const std::string& text)
    {
        std::vector<std::string> words;
        std::string word;
        for (char ch : text)
        {
            if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_')
            {
                word.push_back(ch);
            }
            else
            {
                if (!word.empty())
                {
                    words.push_back(word);
                    word.clear();
                }
            }
        }
        if (!word.empty())
        {
            words.push_back(normalizeWord(word));
        }
        return words;
    }
    static std::string normalizeWord(const std::string& word)
    {
        std::string result;
        for (char ch : word)
        {
            result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        return result;
    }
};
