#include "AST.h"

void AST::parse(std::string f_) {
    // 读取json文件中的数据
    std::ifstream ifs(f_.c_str(), std::ios::in);
    std::string str((std::istreambuf_iterator<char>(ifs)),
                    std::istreambuf_iterator<char>(0));

    if (doc.Parse(str.data()).HasParseError()) {
        std::cout << "error in parse json file." << std::endl;
    }
    if (doc.HasMember("sourceList")) {
        const rapidjson::Value & sl = doc["sourceList"];
        std::cout << sl.IsArray() << std::endl;
    }
    if (doc.HasMember("contracts")) {
        const rapidjson::Value &cs = doc["contracts"];
        std::cout << (cs.GetType() == rapidjson::kObjectType)  << std::endl;
    }
    return;
}