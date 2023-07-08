#include "AST.h"

using namespace rapidjson;



int AST::parse(std::string path_) {
    // 读取json文件中的数据
    std::ifstream ifs(path_.c_str(), std::ios::in);
    std::string str((std::istreambuf_iterator<char>(ifs)),
                    std::istreambuf_iterator<char>(0));
    // 解析json数据
    if (doc.Parse(str.data()).HasParseError()) {
        std::cerr << "error in parse json file." << std::endl;
        return -1;
    }
    // 至此json数据存放在doc中，下一步操作是找到语法树的根节点
    auto node_sources = doc.FindMember("sources");
    if ( node_sources==doc.MemberEnd()) {
        std::cerr << "There is no \"sources\" node in AST(json file)." << std::endl;
        return -1;
    }
    auto node_filename_solc = node_sources->value.FindMember("");

    /*
    if (doc.HasMember("sourceList")) {
        const rapidjson::Value &sl = doc["sourceList"];
        std::cout << "sl is array?: " << sl.IsArray() << std::endl;
        // const rapidjson::Value &mem = sl.GetArray();
        for (rapidjson::Value::ConstValueIterator it = sl.Begin();it!=sl.End();++it) {
            std::cout << (it->IsString() ? it->GetString() : "") << std::endl;
        }
    }
    if (doc.HasMember("contracts")) {
        const rapidjson::Value &cs = doc["contracts"];
        std::cout << "contracts Type: "
                  << (cs.GetType() == rapidjson::kObjectType) << std::endl;
        std::cout << "has member (storage.solc:Storage)?: "
                  << cs.HasMember("storage.solc:Storage") << std::endl;
        rapidjson::Value::ConstMemberIterator sss = cs.FindMember("storage.solc:Storage");
        std::cout << sss->value.IsObject() << std::endl;
    }*/
    return 0;
}