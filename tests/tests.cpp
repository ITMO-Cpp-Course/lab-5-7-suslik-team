#include <algorithm>
#include <catch2/catch_all.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "in_memory_index/document.hpp"
#include "in_memory_index/document_builder.hpp"
#include "in_memory_index/inverted_index.hpp"
#include "index_transaction/index_store.hpp"
#include "index_transaction/result.hpp"
#include "index_transaction/update_transaction.hpp"

using namespace in_memory_index;
using namespace index_transaction;

// Вспомогательные функции
static bool contains(const std::vector<uint64_t>& vec, uint64_t id)
{
    return std::find(vec.begin(), vec.end(), id) != vec.end();
}

static bool contains_document_with_id(const std::vector<Document>& docs, uint64_t id)
{
    return std::find_if(docs.begin(), docs.end(), [id](const Document& doc) { return doc.id == id; }) != docs.end();
}

// ========== Тесты Document ==========

TEST_CASE("Document: default constructor creates empty document", "[document]")
{
    Document doc;
    REQUIRE(doc.id == 0);
    REQUIRE(doc.name.empty());
    REQUIRE(doc.content.empty());
}

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

TEST_CASE("Document: text() returns content as vector", "[document]")
{
    Document doc(1, "file.txt", {"word1", "word2", "word3"});
    auto text = doc.text();
    REQUIRE(text.size() == 3);
    REQUIRE(text[0] == "word1");
    REQUIRE(text[1] == "word2");
    REQUIRE(text[2] == "word3");
}

TEST_CASE("Document: copy constructor works correctly", "[document]")
{
    Document original(1, "original.txt", {"a", "b", "c"});
    Document copy(original);
    REQUIRE(copy.id == original.id);
    REQUIRE(copy.name == original.name);
    REQUIRE(copy.content == original.content);
}

TEST_CASE("Document: move constructor works correctly", "[document]")
{
    Document original(1, "original.txt", {"a", "b", "c"});
    Document moved(std::move(original));
    REQUIRE(moved.id == 1);
    REQUIRE(moved.name == "original.txt");
    REQUIRE(moved.content.size() == 3);
}

TEST_CASE("Document: copy assignment works correctly", "[document]")
{
    Document original(1, "original.txt", {"a", "b"});
    Document copy;
    copy = original;
    REQUIRE(copy.id == original.id);
    REQUIRE(copy.name == original.name);
    REQUIRE(copy.content == original.content);
}

TEST_CASE("Document: move assignment works correctly", "[document]")
{
    Document original(1, "original.txt", {"a", "b"});
    Document moved;
    moved = std::move(original);
    REQUIRE(moved.id == 1);
    REQUIRE(moved.name == "original.txt");
    REQUIRE(moved.content.size() == 2);
}

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

// ========== Тесты DocumentBuilder ==========

TEST_CASE("DocumentBuilder: buildDocument creates document with correct fields", "[document_builder]")
{
    auto doc = DocumentBuilder::buildDocument(5, "test.txt", "hello world");
    REQUIRE(doc.id == 5);
    REQUIRE(doc.name == "test.txt");
    REQUIRE(doc.content.size() == 2);
    REQUIRE(doc.content[0] == "hello");
    REQUIRE(doc.content[1] == "world");
}

TEST_CASE("DocumentBuilder: splitWords splits text by whitespace", "[document_builder]")
{
    auto words = DocumentBuilder::splitWords("one two three");
    REQUIRE(words.size() == 3);
    REQUIRE(words[0] == "one");
    REQUIRE(words[1] == "two");
    REQUIRE(words[2] == "three");
}

TEST_CASE("DocumentBuilder: splitWords handles punctuation", "[document_builder]")
{
    auto words = DocumentBuilder::splitWords("hello, world! test.");
    REQUIRE(words.size() == 3);
    REQUIRE(words[0] == "hello");
    REQUIRE(words[1] == "world");
    REQUIRE(words[2] == "test");
}

TEST_CASE("DocumentBuilder: splitWords keeps underscores in words", "[document_builder]")
{
    auto words = DocumentBuilder::splitWords("hello_world test_code");
    REQUIRE(words.size() == 2);
    REQUIRE(words[0] == "hello_world");
    REQUIRE(words[1] == "test_code");
}

TEST_CASE("DocumentBuilder: splitWords splits on non-alphanumeric chars", "[document_builder]")
{
    auto words = DocumentBuilder::splitWords("hello@world #test$code");
    REQUIRE(words.size() == 4);
    REQUIRE(words[0] == "hello");
    REQUIRE(words[1] == "world");
    REQUIRE(words[2] == "test");
    REQUIRE(words[3] == "code");
}

TEST_CASE("DocumentBuilder: splitWords handles empty string", "[document_builder]")
{
    auto words = DocumentBuilder::splitWords("");
    REQUIRE(words.empty());
}

TEST_CASE("DocumentBuilder: splitWords handles whitespace only", "[document_builder]")
{
    auto words = DocumentBuilder::splitWords("   \t\n   ");
    REQUIRE(words.empty());
}

TEST_CASE("DocumentBuilder: splitWords handles multiple spaces between words", "[document_builder]")
{
    auto words = DocumentBuilder::splitWords("word1    word2   word3");
    REQUIRE(words.size() == 3);
}

TEST_CASE("DocumentBuilder: normalizeWord converts to lowercase", "[document_builder]")
{
    REQUIRE(DocumentBuilder::normalizeWord("HELLO") == "hello");
    REQUIRE(DocumentBuilder::normalizeWord("HeLLo") == "hello");
}

TEST_CASE("DocumentBuilder: normalizeWord keeps lowercase unchanged", "[document_builder]")
{
    REQUIRE(DocumentBuilder::normalizeWord("hello") == "hello");
}

TEST_CASE("DocumentBuilder: normalizeWord handles mixed case", "[document_builder]")
{
    REQUIRE(DocumentBuilder::normalizeWord("AbCdEf") == "abcdef");
}

TEST_CASE("DocumentBuilder: normalizeWord handles numbers", "[document_builder]")
{
    REQUIRE(DocumentBuilder::normalizeWord("test123") == "test123");
}

TEST_CASE("DocumentBuilder: buildDocument normalizes all words", "[document_builder]")
{
    auto doc = DocumentBuilder::buildDocument(1, "file.txt", "HELLO World TEST");
    REQUIRE(doc.content.size() == 3);
    REQUIRE(doc.content[0] == "hello");
    REQUIRE(doc.content[1] == "world");
    REQUIRE(doc.content[2] == "test");
}

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

// ========== Тесты InvertedIndex (с Result<ErrorCode>) ==========

TEST_CASE("InvertedIndex: addDocument with Document adds words to index", "[index]")
{
    InvertedIndex idx;
    Document doc(1, "test.txt", {"hello", "world"});
    auto res = idx.addDocument(doc);
    REQUIRE(res.has_value());
    auto result = idx.search("hello");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 1);
}

TEST_CASE("InvertedIndex: addDocument with id, name, text uses DocumentBuilder", "[index]")
{
    InvertedIndex idx;
    auto res = idx.addDocument(1, "test.txt", "hello world test");
    REQUIRE(res.has_value());
    auto result = idx.search("world");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 1);
}

TEST_CASE("InvertedIndex: search returns all documents containing word", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(Document(1, "", {"hello", "world"})).value();
    idx.addDocument(Document(2, "", {"hello", "cplusplus"})).value();
    idx.addDocument(Document(3, "", {"world"})).value();
    auto resultHello = idx.search("hello");
    REQUIRE(resultHello.size() == 2);
    REQUIRE(contains(resultHello, 1));
    REQUIRE(contains(resultHello, 2));
    auto resultWorld = idx.search("world");
    REQUIRE(resultWorld.size() == 2);
    REQUIRE(contains(resultWorld, 1));
    REQUIRE(contains(resultWorld, 3));
}

TEST_CASE("InvertedIndex: search for non-existing word returns empty", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "file", "apple banana").value();
    auto result = idx.search("nonexistent");
    REQUIRE(result.empty());
}

TEST_CASE("InvertedIndex: getWordOccurrences returns correct frequencies", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "", "hello hello world").value();
    idx.addDocument(2, "", "hello cplusplus").value();
    auto occ = idx.getWordOccurrences("hello");
    REQUIRE(occ.size() == 2);
    REQUIRE(occ[1] == 2);
    REQUIRE(occ[2] == 1);
}

TEST_CASE("InvertedIndex: getWordOccurrences for missing word returns empty", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(Document(1, "", {"hello"})).value();
    auto occ = idx.getWordOccurrences("missing");
    REQUIRE(occ.empty());
}

TEST_CASE("InvertedIndex: removeDocument removes document from index", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "", "apple banana").value();
    idx.addDocument(2, "", "apple cherry").value();
    REQUIRE(idx.documentCount() == 2);
    idx.removeDocument(1).value();
    REQUIRE(idx.documentCount() == 1);
    auto result = idx.search("apple");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 2);
    REQUIRE(idx.search("banana").empty());
}

TEST_CASE("InvertedIndex: remove non-existing document returns error", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(Document(1, "", {"hello"})).value();
    auto res = idx.removeDocument(999);
    REQUIRE(!res.has_value());
    REQUIRE(res.error() == ErrorCode::DocumentNotFound);
    REQUIRE(idx.documentCount() == 1);
    REQUIRE(idx.search("hello").size() == 1);
}

TEST_CASE("InvertedIndex: addDocument with existing ID returns error and keeps original", "[index]")
{
    InvertedIndex idx;
    auto first = idx.addDocument(1, "first", "hello world");
    REQUIRE(first.has_value());
    REQUIRE(idx.documentCount() == 1);
    auto second = idx.addDocument(1, "second", "new words");
    REQUIRE(!second.has_value());
    REQUIRE(second.error() == ErrorCode::DuplicateDocument);
    REQUIRE(idx.documentCount() == 1);
    REQUIRE(idx.search("hello").size() == 1);
    REQUIRE(idx.search("new").empty());
}

TEST_CASE("InvertedIndex: documentCount returns correct number", "[index]")
{
    InvertedIndex idx;
    REQUIRE(idx.documentCount() == 0);
    idx.addDocument(Document(1, "", {"a"})).value();
    REQUIRE(idx.documentCount() == 1);
    idx.addDocument(2, "", "b").value();
    REQUIRE(idx.documentCount() == 2);
    idx.removeDocument(1).value();
    REQUIRE(idx.documentCount() == 1);
    idx.clear();
    REQUIRE(idx.documentCount() == 0);
}

TEST_CASE("InvertedIndex: clear removes all documents and words", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "", "hello world").value();
    idx.addDocument(2, "", "foo bar").value();
    idx.clear();
    REQUIRE(idx.documentCount() == 0);
    REQUIRE(idx.search("hello").empty());
    REQUIRE(idx.getWordOccurrences("world").empty());
}

TEST_CASE("InvertedIndex: duplicate words in document increase count", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "", "word word word").value();
    auto occ = idx.getWordOccurrences("word");
    REQUIRE(occ[1] == 3);
    REQUIRE(idx.search("word").size() == 1);
}

TEST_CASE("InvertedIndex: multiple documents with same word have correct counts", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "", "common common").value();
    idx.addDocument(2, "", "common").value();
    idx.addDocument(3, "", "common common common").value();
    auto occ = idx.getWordOccurrences("common");
    REQUIRE(occ[1] == 2);
    REQUIRE(occ[2] == 1);
    REQUIRE(occ[3] == 3);
    REQUIRE(occ.size() == 3);
}

TEST_CASE("InvertedIndex: search after removal does not keep stale entries", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "", "apple").value();
    idx.addDocument(2, "", "apple").value();
    idx.addDocument(3, "", "banana").value();
    idx.removeDocument(2).value();
    auto result = idx.search("apple");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 1);
    REQUIRE_FALSE(contains(result, 2));
}

TEST_CASE("InvertedIndex: search is case insensitive", "[index]")
{
    InvertedIndex idx;
    idx.addDocument(1, "doc", "Hello WORLD").value();
    REQUIRE(idx.search("hello").size() == 1);
    REQUIRE(idx.search("HELLO").size() == 1);
    REQUIRE(idx.search("World").size() == 1);
    auto occ = idx.getWordOccurrences("HELLO");
    REQUIRE(occ[1] == 1);
}

// ========== Тесты Result ==========

TEST_CASE("Result holds value", "[result]")
{
    Result<int> r = 42;
    REQUIRE(r.has_value());
    REQUIRE(static_cast<bool>(r) == true);
    REQUIRE(r.value() == 42);
}

TEST_CASE("Result holds error", "[result]")
{
    auto r = Result<int>(std::unexpected(ErrorCode::InvalidDocument));
    REQUIRE(!r.has_value());
    REQUIRE(r.error() == ErrorCode::InvalidDocument);
    REQUIRE(static_cast<bool>(r) == false);
}

TEST_CASE("Result value() throws on error", "[result]")
{
    auto r = Result<int>(std::unexpected(ErrorCode::InternalError));
    REQUIRE_THROWS_AS(r.value(), std::bad_expected_access<ErrorCode>);
}

TEST_CASE("Result move constructor transfers state", "[result]")
{
    auto r1 = Result<int>(42);
    auto r2 = std::move(r1);
    REQUIRE(r2.has_value());
    REQUIRE(r2.value() == 42);
}

TEST_CASE("Result copy constructor copies value", "[result]")
{
    auto r1 = Result<int>(42);
    auto r2 = r1;
    REQUIRE(r2.has_value());
    REQUIRE(r2.value() == 42);
}

TEST_CASE("Result works with std::string as T", "[result]")
{
    auto r = Result<std::string>("hello");
    REQUIRE(r.has_value());
    REQUIRE(r.value() == "hello");
    auto e = Result<std::string>(std::unexpected(ErrorCode::InvalidWord));
    REQUIRE(!e.has_value());
    REQUIRE(e.error() == ErrorCode::InvalidWord);
}

TEST_CASE("Result operator bool works in if", "[result]")
{
    auto r = Result<int>(42);
    if (r)
    {
        REQUIRE(r.value() == 42);
    }
    else
    {
        FAIL("Result should be true");
    }
}

TEST_CASE("Result move-initialization moves value", "[result]")
{
    std::string s = "move me";
    auto r = Result<std::string>(std::move(s));
    REQUIRE(r.has_value());
    REQUIRE(r.value() == "move me");
}

// ========== Тесты IndexStore ==========

TEST_CASE("IndexStore: addDocument adds document successfully", "[index_store]")
{
    IndexStore store;
    auto doc = DocumentBuilder::buildDocument(1, "file.txt", "hello world");
    auto result = store.addDocument(doc);
    REQUIRE(result.has_value());
    REQUIRE(store.documentCount() == 1);
}

TEST_CASE("IndexStore: addDocument with duplicate id returns error", "[index_store]")
{
    IndexStore store;
    auto doc1 = DocumentBuilder::buildDocument(1, "first.txt", "content");
    auto doc2 = DocumentBuilder::buildDocument(1, "second.txt", "other content");
    REQUIRE(store.addDocument(doc1).has_value());
    auto result = store.addDocument(doc2);
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == ErrorCode::DuplicateDocument);
    REQUIRE(store.documentCount() == 1);
}

TEST_CASE("IndexStore: removeDocument removes existing document", "[index_store]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc1.txt", "apple banana")).value();
    store.addDocument(DocumentBuilder::buildDocument(2, "doc2.txt", "apple cherry")).value();
    REQUIRE(store.documentCount() == 2);
    auto result = store.removeDocument(1);
    REQUIRE(result.has_value());
    REQUIRE(store.documentCount() == 1);
    auto searchResult = store.search("apple");
    REQUIRE(searchResult.has_value());
    REQUIRE(searchResult.value().size() == 1);
    REQUIRE(contains_document_with_id(searchResult.value(), 2));
}

TEST_CASE("IndexStore: removeDocument for non-existing id returns error", "[index_store]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc1.txt", "hello")).value();
    auto result = store.removeDocument(999);
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == ErrorCode::DocumentNotFound);
    REQUIRE(store.documentCount() == 1);
}

TEST_CASE("IndexStore: search returns documents containing word", "[index_store]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc1.txt", "hello world")).value();
    store.addDocument(DocumentBuilder::buildDocument(2, "doc2.txt", "hello cpp")).value();
    store.addDocument(DocumentBuilder::buildDocument(3, "doc3.txt", "world")).value();
    auto result = store.search("hello");
    REQUIRE(result.has_value());
    const auto& docs = result.value();
    REQUIRE(docs.size() == 2);
    REQUIRE(contains_document_with_id(docs, 1));
    REQUIRE(contains_document_with_id(docs, 2));
    REQUIRE_FALSE(contains_document_with_id(docs, 3));
}

TEST_CASE("IndexStore: search for non-existing word returns empty", "[index_store]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc.txt", "apple banana")).value();
    auto result = store.search("nonexistent");
    REQUIRE(result.has_value());
    REQUIRE(result.value().empty());
}

TEST_CASE("IndexStore: getWordOccurrences returns correct frequencies", "[index_store]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc1.txt", "hello hello world")).value();
    store.addDocument(DocumentBuilder::buildDocument(2, "doc2.txt", "hello cpp")).value();
    auto result = store.getWordOccurrences("hello");
    REQUIRE(result.has_value());
    const auto& freq = result.value();
    REQUIRE(freq.size() == 2);
    REQUIRE(freq.at(1) == 2);
    REQUIRE(freq.at(2) == 1);
}

TEST_CASE("IndexStore: getWordOccurrences for missing word returns empty", "[index_store]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc.txt", "hello world")).value();
    auto result = store.getWordOccurrences("missing");
    REQUIRE(result.has_value());
    REQUIRE(result.value().empty());
}

TEST_CASE("IndexStore: documentCount returns correct number", "[index_store]")
{
    IndexStore store;
    REQUIRE(store.documentCount() == 0);
    store.addDocument(DocumentBuilder::buildDocument(1, "doc1.txt", "a")).value();
    REQUIRE(store.documentCount() == 1);
    store.addDocument(DocumentBuilder::buildDocument(2, "doc2.txt", "b")).value();
    REQUIRE(store.documentCount() == 2);
    store.removeDocument(1).value();
    REQUIRE(store.documentCount() == 1);
    store.clear();
    REQUIRE(store.documentCount() == 0);
}

TEST_CASE("IndexStore: clear removes all documents and words", "[index_store]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc1.txt", "hello world")).value();
    store.addDocument(DocumentBuilder::buildDocument(2, "doc2.txt", "foo bar")).value();
    store.clear();
    REQUIRE(store.documentCount() == 0);
    auto searchRes = store.search("hello");
    REQUIRE(searchRes.has_value());
    REQUIRE(searchRes.value().empty());
    auto occRes = store.getWordOccurrences("world");
    REQUIRE(occRes.has_value());
    REQUIRE(occRes.value().empty());
}

TEST_CASE("IndexStore: multiple documents with same word are all found", "[index_store]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc1.txt", "common")).value();
    store.addDocument(DocumentBuilder::buildDocument(2, "doc2.txt", "common common")).value();
    store.addDocument(DocumentBuilder::buildDocument(3, "doc3.txt", "uncommon")).value();
    auto result = store.search("common");
    REQUIRE(result.has_value());
    const auto& docs = result.value();
    REQUIRE(docs.size() == 2);
    REQUIRE(contains_document_with_id(docs, 1));
    REQUIRE(contains_document_with_id(docs, 2));
}

TEST_CASE("IndexStore: search is case insensitive", "[index_store]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc.txt", "Hello WORLD")).value();
    auto result1 = store.search("hello");
    REQUIRE(result1.has_value());
    REQUIRE(result1.value().size() == 1);
    auto result2 = store.search("HELLO");
    REQUIRE(result2.has_value());
    REQUIRE(result2.value().size() == 1);
    auto result3 = store.search("world");
    REQUIRE(result3.has_value());
    REQUIRE(result3.value().size() == 1);
}

TEST_CASE("IndexStore: after removal, document does not appear", "[index_store]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc1.txt", "apple apple")).value();
    store.addDocument(DocumentBuilder::buildDocument(2, "doc2.txt", "apple banana")).value();
    store.removeDocument(1).value();
    auto searchRes = store.search("apple");
    REQUIRE(searchRes.has_value());
    const auto& docs = searchRes.value();
    REQUIRE(docs.size() == 1);
    REQUIRE(docs[0].id == 2);
    auto occRes = store.getWordOccurrences("apple");
    REQUIRE(occRes.has_value());
    const auto& freq = occRes.value();
    REQUIRE(freq.size() == 1);
    REQUIRE(freq.count(1) == 0);
    REQUIRE(freq.at(2) == 1);
}

TEST_CASE("IndexStore: addDocument with id, name, text overload works", "[index_store]")
{
    IndexStore store;
    auto result = store.addDocument(10, "doc10.txt", "test document");
    REQUIRE(result.has_value());
    REQUIRE(store.documentCount() == 1);
    auto searchRes = store.search("test");
    REQUIRE(searchRes.has_value());
    REQUIRE(searchRes.value().size() == 1);
    REQUIRE(searchRes.value()[0].id == 10);
    REQUIRE(searchRes.value()[0].name == "doc10.txt");
}

// ========== Тесты UpdateTransaction ==========

TEST_CASE("UpdateTransaction: addDocument with Document adds document", "[update_transaction]")
{
    IndexStore store;
    UpdateTransaction tx(store);
    auto doc = DocumentBuilder::buildDocument(1, "file.txt", "hello world");
    auto result = tx.addDocument(doc);
    REQUIRE(result.has_value());
    REQUIRE(store.documentCount() == 1);
    auto searchRes = store.search("hello");
    REQUIRE(searchRes.has_value());
    REQUIRE(searchRes.value().size() == 1);
    REQUIRE(searchRes.value()[0].id == 1);
}

TEST_CASE("UpdateTransaction: addDocument with id, name, text overload adds document", "[update_transaction]")
{
    IndexStore store;
    UpdateTransaction tx(store);
    auto result = tx.addDocument(42, "doc.txt", "hello world");
    REQUIRE(result.has_value());
    REQUIRE(store.documentCount() == 1);
    auto searchRes = store.search("world");
    REQUIRE(searchRes.has_value());
    REQUIRE(searchRes.value().size() == 1);
    REQUIRE(searchRes.value()[0].id == 42);
}

TEST_CASE("UpdateTransaction: addDocument with duplicate id returns error", "[update_transaction]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "existing.txt", "existing content")).value();
    UpdateTransaction tx(store);
    auto result = tx.addDocument(1, "duplicate.txt", "other content");
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == ErrorCode::DuplicateDocument);
    REQUIRE(store.documentCount() == 1);
    auto searchRes = store.search("other");
    REQUIRE(searchRes.has_value());
    REQUIRE(searchRes.value().empty());
}

TEST_CASE("UpdateTransaction: removeDocument removes existing document", "[update_transaction]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc.txt", "hello world")).value();
    UpdateTransaction tx(store);
    auto result = tx.removeDocument(1);
    REQUIRE(result.has_value());
    REQUIRE(store.documentCount() == 0);
    auto searchRes = store.search("hello");
    REQUIRE(searchRes.has_value());
    REQUIRE(searchRes.value().empty());
}

TEST_CASE("UpdateTransaction: removeDocument for non-existing id returns error", "[update_transaction]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc.txt", "hello")).value();
    UpdateTransaction tx(store);
    auto result = tx.removeDocument(999);
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == ErrorCode::DocumentNotFound);
    REQUIRE(store.documentCount() == 1);
}

TEST_CASE("UpdateTransaction: commit persists added documents", "[update_transaction]")
{
    IndexStore store;
    {
        UpdateTransaction tx(store);
        REQUIRE(tx.addDocument(1, "doc.txt", "hello world").has_value());
        REQUIRE(tx.commit().has_value());
    }
    REQUIRE(store.documentCount() == 1);
    auto searchRes = store.search("hello");
    REQUIRE(searchRes.has_value());
    REQUIRE(searchRes.value().size() == 1);
}

TEST_CASE("UpdateTransaction: commit persists removed documents", "[update_transaction]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc.txt", "hello world")).value();
    {
        UpdateTransaction tx(store);
        REQUIRE(tx.removeDocument(1).has_value());
        REQUIRE(tx.commit().has_value());
    }
    REQUIRE(store.documentCount() == 0);
    auto searchRes = store.search("hello");
    REQUIRE(searchRes.has_value());
    REQUIRE(searchRes.value().empty());
}

TEST_CASE("UpdateTransaction: destructor without commit rolls back added documents", "[update_transaction]")
{
    IndexStore store;
    {
        UpdateTransaction tx(store);
        REQUIRE(tx.addDocument(1, "doc.txt", "hello world").has_value());
        REQUIRE(store.documentCount() == 1);
    }
    REQUIRE(store.documentCount() == 0);
    auto searchRes = store.search("hello");
    REQUIRE(searchRes.has_value());
    REQUIRE(searchRes.value().empty());
}

TEST_CASE("UpdateTransaction: destructor without commit rolls back removed documents", "[update_transaction]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc.txt", "hello world")).value();
    {
        UpdateTransaction tx(store);
        REQUIRE(tx.removeDocument(1).has_value());
        REQUIRE(store.documentCount() == 0);
    }
    REQUIRE(store.documentCount() == 1);
    auto searchRes = store.search("hello");
    REQUIRE(searchRes.has_value());
    REQUIRE(searchRes.value().size() == 1);
    REQUIRE(searchRes.value()[0].id == 1);
}

TEST_CASE("UpdateTransaction: rollback of multiple operations happens in reverse order", "[update_transaction]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc1.txt", "apple")).value();
    store.addDocument(DocumentBuilder::buildDocument(2, "doc2.txt", "banana")).value();
    {
        UpdateTransaction tx(store);
        REQUIRE(tx.removeDocument(1).has_value());
        REQUIRE(tx.removeDocument(2).has_value());
        REQUIRE(tx.addDocument(3, "doc3.txt", "cherry").has_value());
        REQUIRE(store.documentCount() == 1);
    }
    REQUIRE(store.documentCount() == 2);
    auto searchApple = store.search("apple");
    REQUIRE(searchApple.has_value());
    REQUIRE(contains_document_with_id(searchApple.value(), 1));
    auto searchBanana = store.search("banana");
    REQUIRE(searchBanana.has_value());
    REQUIRE(contains_document_with_id(searchBanana.value(), 2));
    auto searchCherry = store.search("cherry");
    REQUIRE(searchCherry.has_value());
    REQUIRE(searchCherry.value().empty());
}

TEST_CASE("UpdateTransaction: commit clears rollback log", "[update_transaction]")
{
    IndexStore store;
    UpdateTransaction tx(store);
    REQUIRE(tx.addDocument(1, "doc.txt", "hello").has_value());
    REQUIRE(tx.commit().has_value());
    REQUIRE(store.documentCount() == 1);
}

TEST_CASE("UpdateTransaction: mixed add and remove rollback correctly", "[update_transaction]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(10, "old.txt", "old content")).value();
    {
        UpdateTransaction tx(store);
        REQUIRE(tx.addDocument(20, "new.txt", "new content").has_value());
        REQUIRE(tx.removeDocument(10).has_value());
        REQUIRE(store.documentCount() == 1);
    }
    REQUIRE(store.documentCount() == 1);
    auto oldRes = store.search("old");
    REQUIRE(oldRes.has_value());
    REQUIRE(oldRes.value().size() == 1);
    REQUIRE(oldRes.value()[0].id == 10);
    auto newRes = store.search("new");
    REQUIRE(newRes.has_value());
    REQUIRE(newRes.value().empty());
}

TEST_CASE("UpdateTransaction: multiple addDocuments all rolled back without commit", "[update_transaction]")
{
    IndexStore store;
    {
        UpdateTransaction tx(store);
        REQUIRE(tx.addDocument(1, "a.txt", "alpha").has_value());
        REQUIRE(tx.addDocument(2, "b.txt", "beta").has_value());
        REQUIRE(tx.addDocument(3, "c.txt", "gamma").has_value());
        REQUIRE(store.documentCount() == 3);
    }
    REQUIRE(store.documentCount() == 0);
}

TEST_CASE("UpdateTransaction: multiple addDocuments all committed", "[update_transaction]")
{
    IndexStore store;
    {
        UpdateTransaction tx(store);
        REQUIRE(tx.addDocument(1, "a.txt", "alpha").has_value());
        REQUIRE(tx.addDocument(2, "b.txt", "beta").has_value());
        REQUIRE(tx.addDocument(3, "c.txt", "gamma").has_value());
        REQUIRE(tx.commit().has_value());
    }
    REQUIRE(store.documentCount() == 3);
    auto searchAlpha = store.search("alpha");
    REQUIRE(searchAlpha.has_value());
    REQUIRE(contains_document_with_id(searchAlpha.value(), 1));
    auto searchBeta = store.search("beta");
    REQUIRE(searchBeta.has_value());
    REQUIRE(contains_document_with_id(searchBeta.value(), 2));
    auto searchGamma = store.search("gamma");
    REQUIRE(searchGamma.has_value());
    REQUIRE(contains_document_with_id(searchGamma.value(), 3));
}

TEST_CASE("UpdateTransaction: empty transaction commit is a no-op", "[update_transaction]")
{
    IndexStore store;
    {
        UpdateTransaction tx(store);
        REQUIRE(tx.commit().has_value());
    }
    REQUIRE(store.documentCount() == 0);
}

TEST_CASE("UpdateTransaction: empty transaction destructor without commit is a no-op", "[update_transaction]")
{
    IndexStore store;
    {
        UpdateTransaction tx(store);
    }
    REQUIRE(store.documentCount() == 0);
}

TEST_CASE("UpdateTransaction: failed addDocument is not added to rollback log", "[update_transaction]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc.txt", "original")).value();
    {
        UpdateTransaction tx(store);
        auto result = tx.addDocument(1, "dup.txt", "duplicate");
        REQUIRE(!result.has_value());
        REQUIRE(result.error() == ErrorCode::DuplicateDocument);
    }
    REQUIRE(store.documentCount() == 1);
    auto searchRes = store.search("original");
    REQUIRE(searchRes.has_value());
    REQUIRE(searchRes.value().size() == 1);
    REQUIRE(searchRes.value()[0].id == 1);
}

TEST_CASE("UpdateTransaction: failed removeDocument is not added to rollback log", "[update_transaction]")
{
    IndexStore store;
    store.addDocument(DocumentBuilder::buildDocument(1, "doc.txt", "hello")).value();
    {
        UpdateTransaction tx(store);
        auto result = tx.removeDocument(999);
        REQUIRE(!result.has_value());
        REQUIRE(result.error() == ErrorCode::DocumentNotFound);
    }
    REQUIRE(store.documentCount() == 1);
    auto searchRes = store.search("hello");
    REQUIRE(searchRes.has_value());
    REQUIRE(searchRes.value().size() == 1);
}