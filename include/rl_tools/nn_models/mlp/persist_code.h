#include "../../version.h"
#if (defined(RL_TOOLS_DISABLE_INCLUDE_GUARDS) || !defined(RL_TOOLS_NN_MODELS_MLP_PERSIST_CODE_H)) && (RL_TOOLS_USE_THIS_VERSION == 1)
#pragma once
#define RL_TOOLS_NN_MODELS_MLP_PERSIST_CODE_H



#include <string>
#include <sstream>
#include "../../persist/code.h"
#include "network.h"
RL_TOOLS_NAMESPACE_WRAPPER_START
namespace rl_tools{
    template<typename DEVICE, typename SPEC>
    persist::Code save_code_split(DEVICE& device, nn_models::mlp::NeuralNetworkForward<SPEC>& network, std::string name, bool const_declaration=false, typename DEVICE::index_t indent = 0){
        using T = typename SPEC::T;
        using TI = typename DEVICE::index_t;
        std::stringstream indent_ss;
        for(TI i=0; i < indent; i++){
            indent_ss << "    ";
        }
        std::string ind = indent_ss.str();
        std::stringstream ss_header;
        ss_header << "#include <rl_tools/nn_models/mlp/network.h>\n";
        std::stringstream ss;
        ss << ind << "namespace " << name << " {\n";
        auto input_layer = save_code_split(device, network.input_layer, "input_layer", const_declaration, indent+1);
        ss_header << input_layer.header;
        ss << input_layer.body;
        for(TI hidden_layer_i = 0; hidden_layer_i < SPEC::NUM_HIDDEN_LAYERS; hidden_layer_i++){
            auto hidden_layer = save_code_split(device, network.hidden_layers[hidden_layer_i], "hidden_layer_" + std::to_string(hidden_layer_i), const_declaration, indent+1);
            ss_header << hidden_layer.header;
            ss << hidden_layer.body;
        }
        auto output_layer = save_code_split(device, network.output_layer, "output_layer", const_declaration, indent+1);
        ss_header << output_layer.header;
        std::string T_string = containers::persist::get_type_string<T>();
        std::string TI_string = containers::persist::get_type_string<TI>();
        ss << output_layer.body;
        ss << ind << "    using SPEC = RL_TOOLS""_NAMESPACE_WRAPPER ::rl_tools::nn_models::mlp::Specification<";
        ss << T_string << ", ";
        ss << TI_string << ", ";
        ss << SPEC::INPUT_DIM << ", " << SPEC::OUTPUT_DIM << ", " << SPEC::NUM_LAYERS << ", " << SPEC::HIDDEN_DIM << ", ";
        ss << nn::layers::dense::persist::get_activation_function_string<SPEC::HIDDEN_ACTIVATION_FUNCTION>() << ", ";
        ss << nn::layers::dense::persist::get_activation_function_string<SPEC::OUTPUT_ACTIVATION_FUNCTION>() << ", ";
        ss << "RL_TOOLS""_NAMESPACE_WRAPPER ::rl_tools::nn::layers::dense::DefaultInitializer<" << T_string << ", " << TI_string << ">, ";
        ss << "RL_TOOLS""_NAMESPACE_WRAPPER ::rl_tools::MatrixDynamicTag,";
        ss << "true, ";
        ss << "RL_TOOLS""_NAMESPACE_WRAPPER ::rl_tools::matrix::layouts::RowMajorAlignment<" << containers::persist::get_type_string<TI>() << ", 1>>; \n";
        ss << ind << "    template <typename CAPABILITY>" << "\n";
        ss << ind << "    using TEMPLATE = RL_TOOLS""_NAMESPACE_WRAPPER ::rl_tools::nn_models::mlp::NeuralNetwork<CAPABILITY, SPEC>; \n";
        ss << ind << "    using CAPABILITY = " << to_string(typename SPEC::CAPABILITY{}) << "; \n";
        ss << ind << "    using TYPE = RL_TOOLS""_NAMESPACE_WRAPPER ::rl_tools::nn_models::mlp::NeuralNetwork<CAPABILITY, SPEC>; \n";
        std::stringstream ss_initializer_list;
        ss_initializer_list << "{input_layer::module, ";
        ss_initializer_list << "{";
        for(TI hidden_layer_i = 0; hidden_layer_i < SPEC::NUM_HIDDEN_LAYERS; hidden_layer_i++){
            if(hidden_layer_i > 0){
                ss_initializer_list << ", ";
            }
            ss_initializer_list << "hidden_layer_" << hidden_layer_i << "::module";
        }
        ss_initializer_list << "}, ";
        ss_initializer_list << "output_layer::module}";
        std::string initializer_list;
        if constexpr(SPEC::CAPABILITY::TAG == nn::LayerCapability::Forward){
            initializer_list = "{" + ss_initializer_list.str() + "}";
        }
        else{
            if constexpr(SPEC::CAPABILITY::TAG == nn::LayerCapability::Backward){
                initializer_list = "{{" + ss_initializer_list.str() + "}}";
            }
            else{
                if constexpr(SPEC::CAPABILITY::TAG == nn::LayerCapability::Gradient){
                    initializer_list = "{{{" + ss_initializer_list.str() + "}}}";
                }
                else{
                    utils::assert_exit(device, false, "Unknown capability");
                }
            }
        }
        ss << ind << "    " << (const_declaration ? "const " : "") << "TYPE module = " << initializer_list << ";\n";
        ss << ind << "}\n";
        return {ss_header.str(), ss.str()};
    }
//    template<typename DEVICE, typename SPEC>
//    persist::Code save_code_split(DEVICE& device, nn_models::mlp::NeuralNetworkBackward<SPEC>& network, std::string name, bool const_declaration=false, typename DEVICE::index_t indent = 0){
//        return save_code_split(device, static_cast<nn_models::mlp::NeuralNetworkForward<SPEC>&>(network), name, const_declaration, indent);
//    }
//    template<typename DEVICE, typename SPEC>
//    persist::Code save_code_split(DEVICE& device, nn_models::mlp::NeuralNetworkGradient<SPEC>& network, std::string name, bool const_declaration=false, typename DEVICE::index_t indent = 0){
//        return save_code_split(device, static_cast<nn_models::mlp::NeuralNetworkBackward<SPEC>&>(network), name, const_declaration, indent);
//    }
    template<typename DEVICE, typename SPEC>
    std::string save_code(DEVICE& device, nn_models::mlp::NeuralNetworkForward<SPEC>& network, std::string name, bool const_declaration = true, typename DEVICE::index_t indent = 0) {
        auto code = save_code_split(device, network, name, const_declaration, indent);
        return code.header + code.body;
    }
}
RL_TOOLS_NAMESPACE_WRAPPER_END

#endif
