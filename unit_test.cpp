#include <iostream>
#include <typeinfo>

#include "src/util/any.hpp"
#include "src/util/block_queue.hpp"
#include "src/util/config.hpp"
#include "src/util/json.hpp"
#include "src/util/logger.hpp"
#include "src/util/string_view.hpp"
#include "src/util/time_stamp.hpp"

#include "src/net/event_loop.hpp"
#include "src/net/event_loop_thread.hpp"
#include "src/net/tcp_client.hpp"
#include "src/net/tcp_server.hpp"

using namespace Asuka;
using namespace Net;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define UNIT_TEST_BASE(equality, expect, actual)                        \
    do                                                                  \
    {                                                                   \
        test_count++;                                                   \
        if (equality)                                                   \
            test_pass++;                                                \
        else                                                            \
        {                                                               \
            std::cerr << __FILE__ << ":" << __LINE__ << ":  except: "   \
                << expect << " actual: " << actual << std::endl;        \
            main_ret = 1;                                               \
        }                                                               \
    } while (0)

#define UNIT_TEST(expect, actual) UNIT_TEST_BASE((expect) == (actual), (expect), (actual))

void test_any()
{
    Any any1;
    UNIT_TEST(false, any1.has_value());
    UNIT_TEST(true, any1.type() == typeid(void));
    // any_cast<int>(any1);  // throw bad_any_cast
    
    Any any2{ 42 };
    UNIT_TEST(true, any2.has_value());
    UNIT_TEST(true, any2.type() == typeid(int));
    UNIT_TEST(42, any_cast<int>(any2));

    any1.swap(any2);
    UNIT_TEST(true, any1.has_value());
    UNIT_TEST(42, any_cast<int>(any1));
    UNIT_TEST(false, any2.has_value());

    any1.reset();
    UNIT_TEST(false, any1.has_value());
    
    const Any any3{ 42 };
    UNIT_TEST(42, any_cast<int>(any3));

    Any any4 = any3;
    UNIT_TEST(42, any_cast<int>(std::move(any4)));

    Any any5 = std::move(any4);
    UNIT_TEST(false, any4.has_value());
    UNIT_TEST(42, any_cast<int>(any5));
}

void test_block_queue()
{
    // TODO
}

void test_config()
{
#if 1
#endif
}

void test_log()
{
#if 0
    LOG_TRACE << "abc" << 42 << 88.8;
    LOG_DEBUG << "abc" << 42 << 88.8;
    LOG_INFO << "abc" << 42 << 88.8;
    LOG_WARN << "abc" << 42 << 88.8;
   
    errno = EINTR;
    LOG_ERROR << "abc" << 42 << 88.8;
    LOG_SYSERR << "abc" << 42 << 88.8;
#endif
}

void test_string_view()
{

}

void test_time_stamp()
{
    TimeStamp ts1 = TimeStamp::create_invalid_timestamp();
    UNIT_TEST(false, ts1.is_valid());

    TimeStamp ts2 = TimeStamp::now();
    UNIT_TEST(true, ts2.is_valid());
#if 0
    std::cout << ts2.to_string() << std::endl;
    std::cout << ts2.to_formatted_string() << std::endl;
    std::cout << ts2.to_formatted_string(false) << std::endl;
#endif
}



void test_null()
{
    Json j1;
    UNIT_TEST(true, j1.is_null());
    Json j2 = nullptr;
    UNIT_TEST(true, j2.is_null());

    Json j3 = Json::parse("null");
    UNIT_TEST(true, j3.is_null());
}

void test_boolean()
{
    Json j1 = true;
    UNIT_TEST(true, j1.is_boolean());
    UNIT_TEST("true", j1.dump());
    UNIT_TEST(true, j1.get_value<Json::Bool>());

    Json j2 = false;
    UNIT_TEST(true, j2.is_boolean());
    UNIT_TEST("false", j2.dump());
    UNIT_TEST(false, j2.get_value<Json::Bool>());

    Json j3 = Json::parse("true");
    UNIT_TEST(true, j3.is_boolean());
    UNIT_TEST(true, j3.get_value<Json::Bool>());

    Json j4 = Json::parse("false");
    UNIT_TEST(true, j4.is_boolean());
    UNIT_TEST(false, j4.get_value<Json::Bool>());

    Json j5 = Json::parse("true ");
    UNIT_TEST(true, j5.is_boolean());
    UNIT_TEST(true, j5.get_value<Json::Bool>());
}

#define TEST_NUMBER_VALUE(val)                          \
    do                                                  \
    {                                                   \
        Json j = val;                                   \
        UNIT_TEST(true, j.is_number());                 \
        UNIT_TEST(static_cast<double>(val),             \
            j.get_value<Json::Number>());             \
    } while (0)

#define TEST_NUMBER_PARSE(val, str)                     \
    do                                                  \
    {                                                   \
        Json j = Json::parse(str);                      \
        UNIT_TEST(true, j.is_number());                 \
        UNIT_TEST(val, j.get_value<Json::Number>());  \
    } while (0) 

#define TEST_STRINGIFY(str)                             \
    do                                                  \
    {                                                   \
        Json j = Json::parse(str);                      \
        std::string s1 = j.dump();                      \
        Json j2 = Json::parse(s1);                      \
        std::string s2 = j2.dump();                     \
        UNIT_TEST(s1, s2);                              \
    } while (0)

void test_number()
{
    TEST_NUMBER_VALUE(0);
    TEST_NUMBER_VALUE(1.5);
    TEST_NUMBER_VALUE(1e10);

    TEST_NUMBER_PARSE(0.0, "0");
    TEST_NUMBER_PARSE(0.0, "-0");
    TEST_NUMBER_PARSE(0.0, "-0.0");
    TEST_NUMBER_PARSE(1.0, "1");
    TEST_NUMBER_PARSE(-1.0, "-1");
    TEST_NUMBER_PARSE(1.5, "1.5");
    TEST_NUMBER_PARSE(-1.5, "-1.5");
    TEST_NUMBER_PARSE(3.1416, "3.1416");
    TEST_NUMBER_PARSE(1E10, "1E10");
    TEST_NUMBER_PARSE(1e10, "1e10");
    TEST_NUMBER_PARSE(1E+10, "1E+10");
    TEST_NUMBER_PARSE(1E-10, "1E-10");
    TEST_NUMBER_PARSE(-1E10, "-1E10");
    TEST_NUMBER_PARSE(-1e10, "-1e10");
    TEST_NUMBER_PARSE(-1E+10, "-1E+10");
    TEST_NUMBER_PARSE(-1E-10, "-1E-10");
    TEST_NUMBER_PARSE(1.234E+10, "1.234E+10");
    TEST_NUMBER_PARSE(1.234E-10, "1.234E-10");
    TEST_NUMBER_PARSE(0.0, "1e-10000"); // underflow 

    TEST_NUMBER_PARSE(1.0000000000000002, "1.0000000000000002");            // the smallest number > 1 
    TEST_NUMBER_PARSE(4.9406564584124654e-324, "4.9406564584124654e-324");  // minimum denormal 
    TEST_NUMBER_PARSE(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER_PARSE(2.2250738585072009e-308, "2.2250738585072009e-308");  // max subnormal double 
    TEST_NUMBER_PARSE(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER_PARSE(2.2250738585072014e-308, "2.2250738585072014e-308");  // min normal positive double
    TEST_NUMBER_PARSE(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER_PARSE(1.7976931348623157e+308, "1.7976931348623157e+308");  // max double
    TEST_NUMBER_PARSE(-1.7976931348623157e+308, "-1.7976931348623157e+308");

    TEST_STRINGIFY("0");
    TEST_STRINGIFY("-0");
    TEST_STRINGIFY("1");
    TEST_STRINGIFY("-1");
    TEST_STRINGIFY("1.5");
    TEST_STRINGIFY("-1.5");
    TEST_STRINGIFY("3.25");
    TEST_STRINGIFY("1e+20");
    TEST_STRINGIFY("1.234e+20");
    TEST_STRINGIFY("1.234e-20");

    TEST_STRINGIFY("1.0000000000000002");       // the smallest number > 1
    TEST_STRINGIFY("4.9406564584124654e-324");  // minimum denormal
    TEST_STRINGIFY("-4.9406564584124654e-324");
    TEST_STRINGIFY("2.2250738585072009e-308");  // max subnormal double 
    TEST_STRINGIFY("-2.2250738585072009e-308");
    TEST_STRINGIFY("2.2250738585072014e-308");  // min normal positive double
    TEST_STRINGIFY("-2.2250738585072014e-308");
    TEST_STRINGIFY("1.7976931348623157e+308");  // max double
    TEST_STRINGIFY("-1.7976931348623157e+308");
}

#define TEST_STRING_PARSE(str, jstr)                    \
    do                                                  \
    {                                                   \
        Json j = Json::parse(jstr);                     \
        UNIT_TEST(true, j.is_string());                 \
        UNIT_TEST(str, j.get_value<Json::String>());  \
    } while (0)

void test_string()
{
    TEST_STRING_PARSE("", "\"\"");
    TEST_STRING_PARSE("Hello", "\"Hello\"");
    TEST_STRING_PARSE("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING_PARSE("\"\\/\b\f\n\r\t", "\"\\\"\\\\/\\b\\f\\n\\r\\t\"");
    TEST_STRING_PARSE("\x24", "\"\\u0024\"");                       // Dollar sign U+0024 
    TEST_STRING_PARSE("\xC2\xA2", "\"\\u00A2\"");                   // Cents sign U+00A2 
    TEST_STRING_PARSE("\xE2\x82\xAC", "\"\\u20AC\"");               // Euro sign U+20AC 
    TEST_STRING_PARSE("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");    // G clef sign U+1D11E 
    TEST_STRING_PARSE("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");    // G clef sign U+1D11E 
}

void test_array()
{
    Json j1 = "[ ]"_json;
    UNIT_TEST(true, j1.is_array());
    UNIT_TEST(true, j1.get_value<Json::Array>().empty());

    Json j2 = "[null, true, 42, \"abc\", []]"_json;
    UNIT_TEST(true, j2.is_array());
    UNIT_TEST(5, j2.get_value<Json::Array>().size());
    // cout << j2 << endl;
    UNIT_TEST(true, j2[0].is_null());
    UNIT_TEST(true, j2[1].is_boolean());
    UNIT_TEST(true, j2[1].get_value<Json::Bool>());
    UNIT_TEST(42.0, j2[2].get_value<Json::Number>());
    UNIT_TEST("abc", j2[3].get_value<Json::String>());
    UNIT_TEST(true, j2[4].is_array());

    Json j3 = "[[], [0], [0,1], [0, 1,  2]]"_json;
    UNIT_TEST(true, j3.is_array());
    UNIT_TEST(4, j3.get_value<Json::Array>().size());
    UNIT_TEST(true, j3[0].is_array());
    UNIT_TEST(true, j3[0].get_value<Json::Array>().empty());
    UNIT_TEST(0, j3[0].get_value<Json::Array>().size());
    UNIT_TEST(1, j3[1].get_value<Json::Array>().size());
    UNIT_TEST(2, j3[2].get_value<Json::Array>().size());
    UNIT_TEST(3, j3[3].get_value<Json::Array>().size());

    Json j4 = Json::Array{ nullptr, true, 42, "abc", Json::Array{} };
    UNIT_TEST(true, j4.is_array());
    UNIT_TEST(5, j4.size());
    UNIT_TEST(true, j4[0].is_null());
    UNIT_TEST(true, j4[1].is_boolean());
    UNIT_TEST(true, j4[1].get_value<Json::Bool>());
    UNIT_TEST(42.0, j4[2].get_value<Json::Number>());
    UNIT_TEST("abc", j4[3].get_value<Json::String>());
    UNIT_TEST(true, j4[4].is_array());
}

void test_object()
{
    Json j1 = " { "
        "\"n\" : null , "
        "\"f\" : false , "
        "\"t\" : true , "
        "\"i\" : 123 , "
        "\"s\" : \"abc\", "
        "\"a\" : [1, 2, 3],"
        "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
        " } "_json;

    UNIT_TEST(true, j1.is_object());
    UNIT_TEST(true, j1["n"].is_null());
    UNIT_TEST(false, j1["f"].get_value<Json::Bool>());
    UNIT_TEST(true, j1["t"].get_value<Json::Bool>());
    UNIT_TEST(123.0, j1["i"].get_value<Json::Number>());
    UNIT_TEST("abc", j1["s"].get_value<Json::String>());
    UNIT_TEST(true, j1["a"].is_array());
    UNIT_TEST(3, j1["a"].size());
    UNIT_TEST(1.0, j1["a"][0].get_value<Json::Number>());
    UNIT_TEST(true, j1["o"].is_object());
    UNIT_TEST(1.0, j1["o"]["1"].get_value<Json::Number>());
    UNIT_TEST(3, j1["o"].size());

    j1["f"] = true;
    UNIT_TEST(true, j1["f"].get_value<Json::Bool>());
}

void test_error()
{
    UNIT_TEST(true, Json::parse("nullptr").is_error());
    UNIT_TEST(true, Json::parse("truex").is_error());
    UNIT_TEST(true, Json::parse("+0").is_error());
    UNIT_TEST(true, Json::parse("+1").is_error());
    UNIT_TEST(true, Json::parse("0123").is_error());
    UNIT_TEST(true, Json::parse("0x0").is_error());
    UNIT_TEST(true, Json::parse("0x123").is_error());
    UNIT_TEST(true, Json::parse("1e309").is_error());   // too big
    UNIT_TEST(true, Json::parse("-1e309").is_error());
    UNIT_TEST(true, Json::parse(".123").is_error());
    UNIT_TEST(true, Json::parse("1.").is_error());
    UNIT_TEST(true, Json::parse("INF").is_error());
    UNIT_TEST(true, Json::parse("NaN").is_error());
    UNIT_TEST(true, Json::parse("\"").is_error());
    UNIT_TEST(true, Json::parse("\"abc").is_error());
    UNIT_TEST(true, Json::parse("\"\\v\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\'\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\0\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\x12\"").is_error());
    UNIT_TEST(true, Json::parse("\"\x01\"").is_error());
    UNIT_TEST(true, Json::parse("\"\x1F\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\u\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\u0\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\u01\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\u012\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\u/000\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\uG000\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\u0/00\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\u0G00\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\u0/00\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\u00G0\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\u000/\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\u000G\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\uD800\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\uDBFF\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\uD800\\\\\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\uD800\\uDBFF\"").is_error());
    UNIT_TEST(true, Json::parse("\"\\uD800\\uE000\"").is_error());
    UNIT_TEST(true, Json::parse("[1,").is_error());
    UNIT_TEST(true, Json::parse("[\"a\", nul]").is_error());
    UNIT_TEST(true, Json::parse("[1").is_error());
    UNIT_TEST(true, Json::parse("[1}").is_error());
    UNIT_TEST(true, Json::parse("[1 2").is_error());
    UNIT_TEST(true, Json::parse("[[]").is_error());
    UNIT_TEST(true, Json::parse("{:1,").is_error());
    UNIT_TEST(true, Json::parse("{1:1,").is_error());
    UNIT_TEST(true, Json::parse("{false:1,").is_error());
    UNIT_TEST(true, Json::parse("{null:1,").is_error());
    UNIT_TEST(true, Json::parse("{[]:1,").is_error());
    UNIT_TEST(true, Json::parse("{{}:1,").is_error());
    UNIT_TEST(true, Json::parse("{\"a\":1,").is_error());
    UNIT_TEST(true, Json::parse("{\"a\"}").is_error());
    UNIT_TEST(true, Json::parse("{\"a\",\"b\"}").is_error());
    UNIT_TEST(true, Json::parse("{\"a\":1").is_error());
    UNIT_TEST(true, Json::parse("{\"a\":1]").is_error());
    UNIT_TEST(true, Json::parse("{\"a\":1 \"b\"").is_error());
    UNIT_TEST(true, Json::parse("{\"a\":{}").is_error());
}

void test_json()
{
    test_null();
    test_boolean();
    test_number();
    test_string();
    test_array();
    test_object();
    test_error();
}


void test_all()
{
    test_any();
    test_time_stamp();
    test_json();
    test_log();

    std::cout << test_pass << "/" << test_count
        << " (passed " << test_pass * 100.0 / test_count << "%)" << std::endl;
}

class EchoServer
{
public:
    EchoServer(EventLoop* loop, const IpPort& ipport)
        : mServer(loop, ipport, "EchoServer")
    {
        mServer.set_connection_callback(
            std::bind(&EchoServer::on_conection, this, _1));
        mServer.set_message_callback(
            std::bind(&EchoServer::on_message, this, _1, _2, _3));
    }

    void start()
    {
        mServer.start();
    }

private:
    void on_conection(const TcpConnectionPtr& conn)
    {
        LOG_INFO << "EchoServer - " << conn->get_peer_address().get_ipport() << " -> "
            << conn->get_local_address().get_ipport() << " is "
            << (conn->connected() ? "UP" : "DOWN");
    }

    void on_message(const TcpConnectionPtr& conn,
        Buffer& buf, TimeStamp ts)
    {
        std::string msg(buf.retrieve_all_as_string());
        LOG_INFO << conn->get_name() << " echo " << msg.size() << " bytes, "
            << "data received" << "msg: " << msg;
        conn->send(msg);
    }

private:
    TcpServer mServer;
};

class EchoClient
{
public:
    EchoClient(EventLoop* loop, const IpPort& serverAddr, std::string name)
        : mClient(loop, serverAddr, std::move(name))
    {
        mClient.set_connection_callback(
            std::bind(&EchoClient::on_connection, this, _1));
        mClient.set_message_callback(
            std::bind(&EchoClient::on_message, this, _1, _2, _3));
        mClient.enable_retry();
    }

    void connect()
    {
        mClient.connect();
    }

    void disconnect()
    {
        mClient.disconnect();
    }

    void write(const StringView& msg)
    {
        std::lock_guard<std::mutex> lock{ mMutex };
        std::cout << "write " << msg.size() << " bytes" << std::endl;
        mConn->send(msg);
    }

private:
    void on_connection(const TcpConnectionPtr& conn)
    {
        LOG_INFO << "EchoClient - " << conn->get_peer_address().get_ipport() << " -> "
            << conn->get_local_address().get_ipport() << " is "
            << (conn->connected() ? "UP" : "DOWN");

        std::lock_guard<std::mutex> lock{ mMutex };
        if (conn->connected())
            mConn = conn;
        else
            mConn.reset();
    }

    void on_message(const TcpConnectionPtr& conn,
        Buffer& buf, TimeStamp ts)
    {
        std::string msg = buf.retrieve_all_as_string();
        std::cout << msg << std::endl;
    }

private:
    TcpClient mClient;
    std::mutex mMutex;
    TcpConnectionPtr mConn;
};

void test_server()
{
    LOG_INFO << "echo server";

    std::uint16_t port = Config::instance().get_port();
    IpPort listendAddr{ port };

    EventLoop loop;
    EchoServer server{ &loop, listendAddr };
    server.start();
    loop.loop();
}

void test_client()
{
    IpPort serverAddr("127.0.0.1", 9999);
    EventLoopThread loopThread;

    EchoClient client{ loopThread.startLoop(), serverAddr, "echo client" };
    client.connect();

    std::string line;
    while (std::getline(std::cin, line))
    {
        client.write(line);
    }

    client.disconnect();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2s);
}

int main()
{
    //std::cout << "main thread id = " << std::this_thread::get_id() << std::endl;
    //// Logger::set_level(LogLevel::TRACE);
    //test_client();
    
    test_all();

    return 0;
}