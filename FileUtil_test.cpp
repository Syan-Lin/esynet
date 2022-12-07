#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include "thirdparty/doctest.h"
#include "FileUtil.h"

// it should have a test.txt: {hello world}
// in the save directory of the executable

TEST_CASE("FileUtil_Read_Test"){
    SUBCASE("read exist file") {
        char buf[16] = {};
        FileReader fr(std::filesystem::current_path(), "test.txt");
        REQUIRE(fr.exists());
        CHECK(fr.filesize() == 11);
        CHECK(fr.filename() == "test.txt");

        SUBCASE("read once string") {
            CHECK(fr.read() == "hello world");
            CHECK(fr.current() == 11);
        }

        SUBCASE("read once char*") {
            fr.read(buf, 1024);
            CHECK(std::string(buf) == "hello world");
            CHECK(fr.current() == 11);
        }

        SUBCASE("read muiti char*") {
            std::string data;
            while(fr.current() < fr.filesize()) {
                fr.read(buf, 4);
                data += buf;
                memset(buf, 0, 4);
            }
            CHECK(data == "hello world");
            CHECK(fr.current() == 11);
        }
    }

    SUBCASE("read non-exist file") {
        char buf[16] = {};
        FileReader fr(std::filesystem::current_path(), "non-exist.txt");
        REQUIRE(!fr.exists());
        CHECK(fr.filesize() == -1);
        CHECK(fr.filename() == "non-exist.txt");

        SUBCASE("read once string") {
            CHECK(fr.read() == "");
            CHECK(fr.current() == 0);
        }

        SUBCASE("read once char*") {
            fr.read(buf, 1024);
            CHECK(std::string(buf) == "");
            CHECK(fr.current() == 0);
        }

        SUBCASE("read muiti char*") {
            std::string data;
            while(fr.current() < fr.filesize()) {
                INFO("in loop");
                fr.read(buf, 4);
                data += buf;
                memset(buf, 0, 4);
            }
            CHECK(data == "");
            CHECK(fr.current() == 0);
        }
    }
}

TEST_CASE("FileUtil_Write_Test"){
    SUBCASE("write exist file") {
        std::string data = "hello world";
        FileWriter fr(std::filesystem::current_path(), "test.txt");
        REQUIRE(fr.exists());
        CHECK(fr.filesize() == -1);
        CHECK(fr.filename() == "test.txt");
        fr.open();
        CHECK(fr.filesize() == 11);

        fr.append(data.c_str(), data.size());
        CHECK(fr.filesize() == 22);

        // test.txt should have added "hello world"
    }

    SUBCASE("write non-exist file") {
        std::string data = "hello world";
        FileWriter fr(std::filesystem::current_path(), "non-exist.txt");
        REQUIRE(!fr.exists());
        CHECK(fr.filesize() == -1);
        CHECK(fr.filename() == "non-exist.txt");
        fr.open(); // will create a file
        CHECK(fr.filesize() == 0);

        fr.append(data.c_str(), data.size());
        CHECK(fr.filesize() == 11);
    }
}