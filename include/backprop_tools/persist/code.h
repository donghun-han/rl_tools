#include "../version.h"
#if (defined(BACKPROP_TOOLS_DISABLE_INCLUDE_GUARDS) || !defined(BACKPROP_TOOLS_PERSIST_CODE_H)) && (BACKPROP_TOOLS_USE_THIS_VERSION == 1)
#pragma once
#define BACKPROP_TOOLS_PERSIST_CODE_H

#include <sstream>

BACKPROP_TOOLS_NAMESPACE_WRAPPER_START
namespace backprop_tools::persist{
    struct Code{
        std::string header;
        std::string body;
    };
}
BACKPROP_TOOLS_NAMESPACE_WRAPPER_END

BACKPROP_TOOLS_NAMESPACE_WRAPPER_START
namespace backprop_tools{
    template <typename DEVICE>
    persist::Code embed_in_namespace(DEVICE&, persist::Code c, std::string name, typename DEVICE::index_t indent = 0){
        using TI = typename DEVICE::index_t;
        std::stringstream indent_ss;
        for(TI i=0; i < indent; i++){
            indent_ss << "    ";
        }
        std::string ind = indent_ss.str();
        std::stringstream ss_header;
        std::stringstream ss;
        ss_header << c.header;
        ss << ind << "namespace " << name << " {\n";
        ss << c.body;
        ss << ind << "}\n";
        return {ss_header.str(), ss.str()};
    }
}
BACKPROP_TOOLS_NAMESPACE_WRAPPER_END

#endif