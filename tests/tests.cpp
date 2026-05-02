#include <algorithm>
#include <catch2/catch_all.hpp>
#include <in_memory_index/document.hpp>
#include <in_memory_index/document_builder.hpp>
#include <in_memory_index/inverted_index.hpp>

using namespace in_memory_index;

// ========== Вспомогательная функция для проверки наличия ID в векторе ==========
static bool contains(const std::vector<DocId>& vec, DocId id)
{
    return std::find(vec.begin(), vec.end(), id) != vec.end();
}

// ========== Тесты ==========

// Проверяет: базовое добавление одного документа и поиск по слову.
TEST_CASE("InvertedIndex: addDocument and search basic", "[index]")
{
    InvertedIndex idx;

    SECTION("Single document")
    {
        idx.addDocument(1, {"hello", "world"});
        auto result = idx.search("hello");
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 1);
    }

    SECTION("Multiple documents with overlapping words")
    {
        idx.addDocument(1, {"hello", "world"});
        idx.addDocument(2, {"hello", "cplusplus"});
        idx.addDocument(3, {"world"});

        auto resultHello = idx.search("hello");
        REQUIRE(resultHello.size() == 2);
        REQUIRE(contains(resultHello, 1));
        REQUIRE(contains(resultHello, 2));
        REQUIRE_FALSE(contains(resultHello, 3));

        auto resultWorld = idx.search("world");
        REQUIRE(resultWorld.size() == 2);
        REQUIRE(contains(resultWorld, 1));
        REQUIRE(contains(resultWorld, 3));
    }
}

// Проверяет: поиск по несуществующему слову возвращает пустой вектор.
TEST_CASE("InvertedIndex: search for non-existing word returns empty", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, {"apple", "banana"});
    auto result = idx.search("nonexistent");
    REQUIRE(result.empty());
}

// Проверяет: getWordOccurrences возвращает правильные частоты слов для документов.
TEST_CASE("InvertedIndex: getWordOccurrences returns correct frequencies", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, {"hello", "hello", "world"}); // "hello" 2 раза
    idx.addDocument(2, {"hello", "cplusplus"});      // "hello" 1 раз

    auto occ = idx.getWordOccurrences("hello");
    REQUIRE(occ.size() == 2);
    REQUIRE(occ[1] == 2);
    REQUIRE(occ[2] == 1);

    auto occMissing = idx.getWordOccurrences("missing");
    REQUIRE(occMissing.empty());
}

// Проверяет: удаление документа полностью очищает его из индекса и обратной связи.
TEST_CASE("InvertedIndex: removeDocument removes document from all structures", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, {"apple", "banana"});
    idx.addDocument(2, {"apple", "cherry"});

    REQUIRE(idx.documentCount() == 2);
    idx.removeDocument(1);
    REQUIRE(idx.documentCount() == 1);

    auto result = idx.search("apple");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 2);
    REQUIRE_FALSE(contains(idx.search("banana"), 1)); // пустой вектор, так как нет документов с banana
}

// Проверяет: попытка удалить несуществующий документ не влияет на индекс.
TEST_CASE("InvertedIndex: removing non-existing document does nothing", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, {"hello"});
    idx.removeDocument(999); // не существует
    REQUIRE(idx.documentCount() == 1);
    REQUIRE(idx.search("hello").size() == 1);
}

// Проверяет: добавление документа с уже существующим ID перезаписывает старый документ.
TEST_CASE("InvertedIndex: adding document with existing ID replaces it", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, {"hello", "world"});
    idx.addDocument(1, {"new", "words"}); // перезапись

    REQUIRE(idx.documentCount() == 1);
    auto result = idx.search("hello");
    REQUIRE(result.empty()); // старые слова удалены
    result = idx.search("new");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 1);
}

// Проверяет: documentCount возвращает корректное количество документов.
TEST_CASE("InvertedIndex: documentCount returns correct number", "[index]")
{
    InvertedIndex idx;
    REQUIRE(idx.documentCount() == 0);
    idx.addDocument(1, {"a"});
    REQUIRE(idx.documentCount() == 1);
    idx.addDocument(2, {"b"});
    REQUIRE(idx.documentCount() == 2);
    idx.removeDocument(1);
    REQUIRE(idx.documentCount() == 1);
    idx.clear();
    REQUIRE(idx.documentCount() == 0);
}

// Проверяет: метод clear полностью очищает индекс (удаляет все документы и слова).
TEST_CASE("InvertedIndex: clear removes all data", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, {"hello", "world"});
    idx.addDocument(2, {"foo", "bar"});
    idx.clear();
    REQUIRE(idx.documentCount() == 0);
    REQUIRE(idx.search("hello").empty());
    REQUIRE(idx.getWordOccurrences("world").empty());
}

// Проверяет: повторяющиеся слова в одном документе корректно увеличивают счётчик.
TEST_CASE("InvertedIndex: duplicate words in a document are handled correctly", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, {"word", "word", "word"}); // 3 раза
    auto occ = idx.getWordOccurrences("word");
    REQUIRE(occ[1] == 3);
    REQUIRE(idx.search("word").size() == 1);
}

// Проверяет: разные документы с разной частотой одного слова — частоты сохраняются правильно.
TEST_CASE("InvertedIndex: multiple documents with same word and different counts", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, {"common", "common"});
    idx.addDocument(2, {"common"});
    idx.addDocument(3, {"common", "common", "common"});
    auto occ = idx.getWordOccurrences("common");
    REQUIRE(occ[1] == 2);
    REQUIRE(occ[2] == 1);
    REQUIRE(occ[3] == 3);
    REQUIRE(occ.size() == 3);
}

// Проверяет: после удаления некоторых документов, их ID не остаются в поиске по слову.
TEST_CASE("InvertedIndex: search after removal of some documents does not keep stale entries", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, {"apple"});
    idx.addDocument(2, {"apple"});
    idx.addDocument(3, {"banana"});
    idx.removeDocument(2);
    auto result = idx.search("apple");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 1);
    REQUIRE_FALSE(contains(result, 2));
}

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