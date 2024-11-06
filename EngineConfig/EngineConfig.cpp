#include "EngineConfig.hpp"

bool EngineConfig::loadFromFile(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Unable to open file: " << filename << std::endl;
    return false;
  }

  try {
    // Parse the JSON file
    nlohmann::json jsonConfig;
    file >> jsonConfig;

    // Map JSON values to struct members
    cylinder = {
        .stroke = MMToM(
            jsonConfig.at("geometry").at("cylinder").at("stroke").get<float>()),
        .bore = MMToM(
            jsonConfig.at("geometry").at("cylinder").at("bore").get<float>()),
        .add_stroke = MMToM(jsonConfig.at("geometry")
                                .at("cylinder")
                                .at("add_stroke")
                                .get<float>())};

    rod.length =
        MMToM(jsonConfig.at("geometry").at("rod").at("length").get<float>());

    crankshaft.weight =
        jsonConfig.at("geometry").at("crankshaft").at("weight").get<float>();

    intakeValve = {
        .timing =
            jsonConfig.at("valves").at("intake").at("timing").get<float>(),
        .shape = jsonConfig.at("valves").at("intake").at("shape").get<float>(),
    };

    exhaustValve = {
        .timing =
            jsonConfig.at("valves").at("exhaust").at("timing").get<float>(),
        .shape = jsonConfig.at("valves").at("exhaust").at("shape").get<float>(),
    };

  } catch (const nlohmann::json::exception& e) {
    std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    return false;
  }

  return true;
}
