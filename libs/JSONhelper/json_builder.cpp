#include "json_builder.h"
#include <exception>
#include <variant>
#include <utility>

using namespace std::literals;

namespace json {

// ------------------------ Builder ----------------------------------------
Builder::Builder() : root_(), nodes_stack_{&root_} { }

Node Builder::Build() {
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Attempt to build JSON which isn't finalized"s);
    }
    return std::move(root_);
}

Builder::DictValueContext Builder::Key(std::string key) {
    Node::Value& host_value = GetCurrentValue();
    
    if (!std::holds_alternative<Dict>(host_value)) {
        throw std::logic_error("Key() outside a dict"s);
    }
    
    nodes_stack_.push_back(&std::get<Dict>(host_value)[std::move(key)]);
    return BaseContext{*this};
}

Builder::BaseContext Builder::Value(Node::Value value) {
    AddNode(std::move(value));
    return *this;
}

Builder::DictItemContext Builder::StartDict() {
    AddValue(Dict{});
    return BaseContext{*this};
}

Builder::ArrayItemContext Builder::StartArray() {
    AddValue(Array{});
    return BaseContext{*this};
}

Builder::BaseContext Builder::EndDict() {
    if (!std::holds_alternative<Dict>(GetCurrentValue())) {
        throw std::logic_error("EndDict() outside a dict"s);
    }

    nodes_stack_.pop_back();
    return *this;
}

Builder::BaseContext Builder::EndArray() {
    if (!std::holds_alternative<Array>(GetCurrentValue())) {
        throw std::logic_error("EndDict() outside an array"s);
    }

    nodes_stack_.pop_back();
    return *this;
}
    
Node::Value& Builder::GetCurrentValue() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Attempt to change finalized JSON"s);
    }

    return nodes_stack_.back()->GetValue();
}

const Node::Value& Builder::GetCurrentValue() const {
    return const_cast<Builder*>(this)->GetCurrentValue();
}

void Builder::AssertNewObjectContext() const {
    if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue())) {
        throw std::logic_error("New object in wrong context"s);
    }
}

void Builder::AddValue(Node::Value value) {
    Node::Value& host_value = GetCurrentValue();

    if (std::holds_alternative<Array>(host_value)) {
        std::get<Array>(host_value).emplace_back(std::move(value));
    } else if (std::holds_alternative<std::nullptr_t>(host_value)) {
        host_value = std::move(value);
    } else {
        throw std::logic_error("Value already set for current node");
    }
}

void Builder::AddNode(Node::Value value) {
    AssertNewObjectContext();

    Node::Value& host_value = GetCurrentValue();
    host_value = std::move(value);
    
    nodes_stack_.pop_back();
}


// ------------------------ BaseContext ------------------------------------
Builder::BaseContext::BaseContext(Builder& builder) : builder_(builder) {}
Node Builder::BaseContext::Build() { return builder_.Build(); }
Builder::DictValueContext Builder::BaseContext::Key(std::string key) { return builder_.Key(std::move(key)); }
Builder::BaseContext Builder::BaseContext::Value(Node::Value value) { return builder_.Value(std::move(value)); }
Builder::DictItemContext Builder::BaseContext::StartDict() { return builder_.StartDict(); }
Builder::ArrayItemContext Builder::BaseContext::StartArray() { return builder_.StartArray(); }
Builder::BaseContext Builder::BaseContext::EndDict() { return builder_.EndDict(); }   
Builder::BaseContext Builder::BaseContext::EndArray() { return builder_.EndArray(); }

// ------------------------ DictValueContext -------------------------------
Builder::DictValueContext::DictValueContext(BaseContext base) : BaseContext(base) { }
Builder::DictItemContext Builder::DictValueContext::Value(Node::Value value) { return BaseContext::Value(std::move(value)); }

// ------------------------ DictItemContext --------------------------------
Builder::DictItemContext::DictItemContext(BaseContext base) : BaseContext(base) {}

// ------------------------ ArrayItemContext -------------------------------
Builder::ArrayItemContext::ArrayItemContext(BaseContext base) : BaseContext(base) {}
Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
    builder_.AddValue(std::move(value));
    return *this;
}
}  // namespace json