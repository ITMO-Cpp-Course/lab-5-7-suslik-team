#include <catch2/catch_all.hpp>
#include <in_memory_index/document.hpp>
#include <in_memory_index/document_builder.hpp>
#include <in_memory_index/inverted_index.hpp>
#include <string>
#include <vector>

using namespace in_memory_index;

// Вспомогательная функция для проверки наличия ID в векторе
static bool contains(const std::vector<DocId>& vec, DocId id)
{
    return std::find(vec.begin(), vec.end(), id) != vec.end();
}

// ========== Тесты InvertedIndex ==========

// Проверяет: addDocument(Document) корректно добавляет документ в индекс.
TEST_CASE("InvertedIndex: addDocument with Document adds words to index", "[index]")
{
    InvertedIndex idx;
    Document doc(1, "test.txt", {"hello", "world"});
    idx.addDocument(doc);
    auto result = idx.search("hello");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 1);
}

// Проверяет: addDocument(id, name, text) создаёт документ через DocumentBuilder и индексирует слова.
TEST_CASE("InvertedIndex: addDocument with id, name, text uses DocumentBuilder", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "test.txt", "hello world test");
    auto result = idx.search("world");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 1);
}

// Проверяет: search возвращает все документы, содержащие искомое слово.
TEST_CASE("InvertedIndex: search returns all documents containing word", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(Document(1, "", {"hello", "world"}));
    idx.addDocument(Document(2, "", {"hello", "cplusplus"}));
    idx.addDocument(Document(3, "", {"world"}));

    auto resultHello = idx.search("hello");
    REQUIRE(resultHello.size() == 2);
    REQUIRE(contains(resultHello, 1));
    REQUIRE(contains(resultHello, 2));

    auto resultWorld = idx.search("world");
    REQUIRE(resultWorld.size() == 2);
    REQUIRE(contains(resultWorld, 1));
    REQUIRE(contains(resultWorld, 3));
}

// Проверяет: search по несуществующему слову возвращает пустой вектор.
TEST_CASE("InvertedIndex: search for non-existing word returns empty", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "file", "apple banana");
    auto result = idx.search("nonexistent");
    REQUIRE(result.empty());
}

// Проверяет: getWordOccurrences возвращает частоту слова в каждом документе.
TEST_CASE("InvertedIndex: getWordOccurrences returns correct frequencies", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "", "hello hello world");   // hello=2, world=1
    idx.addDocument(2, "", "hello cplusplus");
    auto occ = idx.getWordOccurrences("hello");
    REQUIRE(occ.size() == 2);
    REQUIRE(occ[1] == 2);
    REQUIRE(occ[2] == 1);
}

// Проверяет: getWordOccurrences для отсутствующего слова возвращает пустую map.
TEST_CASE("InvertedIndex: getWordOccurrences for missing word returns empty", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(Document(1, "", {"hello"}));
    auto occ = idx.getWordOccurrences("missing");
    REQUIRE(occ.empty());
}

// Проверяет: removeDocument полностью удаляет документ из индекса.
TEST_CASE("InvertedIndex: removeDocument removes document from index", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "", "apple banana");
    idx.addDocument(2, "", "apple cherry");
    REQUIRE(idx.documentCount() == 2);
    idx.removeDocument(1);
    REQUIRE(idx.documentCount() == 1);
    auto result = idx.search("apple");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 2);
    REQUIRE(idx.search("banana").empty());
}

// Проверяет: удаление несуществующего документа не влияет на индекс.
TEST_CASE("InvertedIndex: remove non-existing document does nothing", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(Document(1, "", {"hello"}));
    idx.removeDocument(999);
    REQUIRE(idx.documentCount() == 1);
    REQUIRE(idx.search("hello").size() == 1);
}

// Проверяет: добавление документа с существующим ID перезаписывает старый.
TEST_CASE("InvertedIndex: addDocument with existing ID replaces document", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "first", "hello world");
    idx.addDocument(1, "second", "new words");
    REQUIRE(idx.documentCount() == 1);
    REQUIRE(idx.search("hello").empty());
    auto result = idx.search("new");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 1);
}

// Проверяет: documentCount возвращает актуальное количество документов.
TEST_CASE("InvertedIndex: documentCount returns correct number", "[index]")
{
    InvertedIndex idx;
    REQUIRE(idx.documentCount() == 0);
    idx.addDocument(Document(1, "", {"a"}));
    REQUIRE(idx.documentCount() == 1);
    idx.addDocument(2, "", "b");
    REQUIRE(idx.documentCount() == 2);
    idx.removeDocument(1);
    REQUIRE(idx.documentCount() == 1);
    idx.clear();
    REQUIRE(idx.documentCount() == 0);
}

// Проверяет: clear полностью очищает индекс от всех документов и слов.
TEST_CASE("InvertedIndex: clear removes all documents and words", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "", "hello world");
    idx.addDocument(2, "", "foo bar");
    idx.clear();
    REQUIRE(idx.documentCount() == 0);
    REQUIRE(idx.search("hello").empty());
    REQUIRE(idx.getWordOccurrences("world").empty());
}

// Проверяет: повторяющиеся слова в документе корректно подсчитываются.
TEST_CASE("InvertedIndex: duplicate words in document increase count", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "", "word word word");
    auto occ = idx.getWordOccurrences("word");
    REQUIRE(occ[1] == 3);
    REQUIRE(idx.search("word").size() == 1);
}

// Проверяет: разные документы с разной частотой слова сохраняют свои счётчики.
TEST_CASE("InvertedIndex: multiple documents with same word have correct counts", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "", "common common");
    idx.addDocument(2, "", "common");
    idx.addDocument(3, "", "common common common");
    auto occ = idx.getWordOccurrences("common");
    REQUIRE(occ[1] == 2);
    REQUIRE(occ[2] == 1);
    REQUIRE(occ[3] == 3);
    REQUIRE(occ.size() == 3);
}

// Проверяет: после удаления документа его id не остаётся в результатах поиска.
TEST_CASE("InvertedIndex: search after removal does not keep stale entries", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "", "apple");
    idx.addDocument(2, "", "apple");
    idx.addDocument(3, "", "banana");
    idx.removeDocument(2);
    auto result = idx.search("apple");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 1);
    REQUIRE_FALSE(contains(result, 2));
}

// Проверяет: поиск и частоты регистронезависимы благодаря нормализации слов.
TEST_CASE("InvertedIndex: search is case insensitive through normalization", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "doc", "Hello WORLD");
    REQUIRE(idx.search("hello").size() == 1);
    REQUIRE(idx.search("HELLO").size() == 1);
    REQUIRE(idx.search("World").size() == 1);
    auto occ = idx.getWordOccurrences("HELLO");
    REQUIRE(occ[1] == 1);
}

// ========== Тесты Document остаются без изменений (приведены для полноты) ==========
// ... (все оригинальные тесты для Document и DocumentBuilder из исходного файла)

// Проверяет: конструктор по умолчанию создаёт документ с пустыми полями.
TEST_CASE("Document: default constructor creates empty document", "[document]")
{
    Document doc;
    REQUIRE(doc.id == 0);
    REQUIRE(doc.name.empty());
    REQUIRE(doc.content.empty());
}

// Проверяет: конструктор с параметрами корректно инициализирует все поля.
TEST_CASE("Document: parameterized constructor sets all fields", "[document]")
{
    std::vector<std::string> words = {"hello", "world"};
    Document doc(42, "test.txt", words);

    REQUIRE(doc.id == 42);
    REQUIRE(doc.name == "test.txt");
    REQUIRE(doc.content.size() == 2);
    REQUIRE(doc.content[0] == "hello");
    REQUIRE(doc.content[1] == "world");
}

// Проверяет: метод text() возвращает содержимое документа в виде вектора строк.
TEST_CASE("Document: text() returns content as vector", "[document]")
{
    Document doc(1, "file.txt", {"word1", "word2", "word3"});
    auto text = doc.text();

    REQUIRE(text.size() == 3);
    REQUIRE(text[0] == "word1");
    REQUIRE(text[1] == "word2");
    REQUIRE(text[2] == "word3");
}

// Проверяет: конструктор копирования создаёт независимую копию объекта.
TEST_CASE("Document: copy constructor works correctly", "[document]")
{
    Document original(1, "original.txt", {"a", "b", "c"});
    Document copy(original);

    REQUIRE(copy.id == original.id);
    REQUIRE(copy.name == original.name);
    REQUIRE(copy.content == original.content);
}

// Проверяет: конструктор перемещения переносит данные без копирования.
TEST_CASE("Document: move constructor works correctly", "[document]")
{
    Document original(1, "original.txt", {"a", "b", "c"});
    Document moved(std::move(original));

    REQUIRE(moved.id == 1);
    REQUIRE(moved.name == "original.txt");
    REQUIRE(moved.content.size() == 3);
}

// Проверяет: оператор присваивания копированием корректно копирует данные.
TEST_CASE("Document: copy assignment works correctly", "[document]")
{
    Document original(1, "original.txt", {"a", "b"});
    Document copy;
    copy = original;

    REQUIRE(copy.id == original.id);
    REQUIRE(copy.name == original.name);
    REQUIRE(copy.content == original.content);
}

// Проверяет: оператор присваивания перемещением переносит данные.
TEST_CASE("Document: move assignment works correctly", "[document]")
{
    Document original(1, "original.txt", {"a", "b"});
    Document moved;
    moved = std::move(original);

    REQUIRE(moved.id == 1);
    REQUIRE(moved.name == "original.txt");
    REQUIRE(moved.content.size() == 2);
}

// Проверяет: копия документа независима от оригинала (глубокое копирование).
TEST_CASE("Document: modifying copy does not affect original", "[document]")
{
    Document original(1, "original.txt", {"a", "b"});
    Document copy(original);
    copy.id = 999;
    copy.name = "changed.txt";
    copy.content = {"x", "y"};

    REQUIRE(original.id == 1);
    REQUIRE(original.name == "original.txt");
    REQUIRE(original.content.size() == 2);
}

// Проверяет: buildDocument создаёт Document с правильными полями из сырого текста.
TEST_CASE("DocumentBuilder: buildDocument creates document with correct fields", "[document_builder]")
{
    auto doc = DocumentBuilder::buildDocument(5, "test.txt", "hello world");

    REQUIRE(doc.id == 5);
    REQUIRE(doc.name == "test.txt");
    REQUIRE(doc.content.size() == 2);
    REQUIRE(doc.content[0] == "hello");
    REQUIRE(doc.content[1] == "world");
}

// Проверяет: splitWords разбивает текст по пробельным символам.
TEST_CASE("DocumentBuilder: splitWords splits text by whitespace", "[document_builder]")
{
    auto words = DocumentBuilder::splitWords("one two three");

    REQUIRE(words.size() == 3);
    REQUIRE(words[0] == "one");
    REQUIRE(words[1] == "two");
    REQUIRE(words[2] == "three");
}

// Проверяет: splitWords корректно обрабатывает знаки препинания.
TEST_CASE("DocumentBuilder: splitWords handles punctuation", "[document_builder]")
{
    auto words = DocumentBuilder::splitWords("hello, world! test.");

    REQUIRE(words.size() == 3);
    REQUIRE(words[0] == "hello");
    REQUIRE(words[1] == "world");
    REQUIRE(words[2] == "test");
}

// Проверяет: splitWords сохраняет символ подчёркивания в словах.
TEST_CASE("DocumentBuilder: splitWords keeps underscores in words", "[document_builder]")
{
    auto words = DocumentBuilder::splitWords("hello_world test_code");

    REQUIRE(words.size() == 2);
    REQUIRE(words[0] == "hello_world");
    REQUIRE(words[1] == "test_code");
}

// Проверяет: splitWords разделяет слова по не буквенно-цифровым символам.
TEST_CASE("DocumentBuilder: splitWords splits on non-alphanumeric chars", "[document_builder]")
{
    auto words = DocumentBuilder::splitWords("hello@world #test$code");

    REQUIRE(words.size() == 4);
    REQUIRE(words[0] == "hello");
    REQUIRE(words[1] == "world");
    REQUIRE(words[2] == "test");
    REQUIRE(words[3] == "code");
}

// Проверяет: splitWords корректно обрабатывает пустую строку.
TEST_CASE("DocumentBuilder: splitWords handles empty string", "[document_builder]")
{
    auto words = DocumentBuilder::splitWords("");

    REQUIRE(words.empty());
}

// Проверяет: splitWords корректно обрабатывает строку только с пробелами.
TEST_CASE("DocumentBuilder: splitWords handles whitespace only", "[document_builder]")
{
    auto words = DocumentBuilder::splitWords("   \t\n   ");

    REQUIRE(words.empty());
}

// Проверяет: splitWords корректно обрабатывает несколько пробелов подряд.
TEST_CASE("DocumentBuilder: splitWords handles multiple spaces between words", "[document_builder]")
{
    auto words = DocumentBuilder::splitWords("word1    word2   word3");

    REQUIRE(words.size() == 3);
}

// Проверяет: normalizeWord преобразует символы в нижний регистр.
TEST_CASE("DocumentBuilder: normalizeWord converts to lowercase", "[document_builder]")
{
    auto result = DocumentBuilder::normalizeWord("HELLO");
    REQUIRE(result == "hello");

    result = DocumentBuilder::normalizeWord("HeLLo");
    REQUIRE(result == "hello");
}

// Проверяет: normalizeWord оставляет уже нижний регистр без изменений.
TEST_CASE("DocumentBuilder: normalizeWord keeps lowercase unchanged", "[document_builder]")
{
    auto result = DocumentBuilder::normalizeWord("hello");
    REQUIRE(result == "hello");
}

// Проверяет: normalizeWord корректно обрабатывает смешанный регистр.
TEST_CASE("DocumentBuilder: normalizeWord handles mixed case", "[document_builder]")
{
    auto result = DocumentBuilder::normalizeWord("AbCdEf");
    REQUIRE(result == "abcdef");
}

// Проверяет: normalizeWord сохраняет цифры в слове.
TEST_CASE("DocumentBuilder: normalizeWord handles numbers", "[document_builder]")
{
    auto result = DocumentBuilder::normalizeWord("test123");
    REQUIRE(result == "test123");
}

// Проверяет: buildDocument нормализует все стова в тексте.
TEST_CASE("DocumentBuilder: buildDocument normalizes all words", "[document_builder]")
{
    auto doc = DocumentBuilder::buildDocument(1, "file.txt", "HELLO World TEST");

    REQUIRE(doc.content.size() == 3);
    REQUIRE(doc.content[0] == "hello");
    REQUIRE(doc.content[1] == "world");
    REQUIRE(doc.content[2] == "test");
}

// Проверяет: splitWords корректно обрабатывает различные спецсимволы.
TEST_CASE("DocumentBuilder: splitWords handles special characters", "[document_builder]")
{
    auto words = DocumentBuilder::splitWords("file.txt path/to/file");

    REQUIRE(words.size() == 5);
    REQUIRE(words[0] == "file");
    REQUIRE(words[1] == "txt");
    REQUIRE(words[2] == "path");
    REQUIRE(words[3] == "to");
    REQUIRE(words[4] == "file");
}