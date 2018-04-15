#pragma once

#include <string>
#include <regex>
#include "StringComparators.hpp"

//The XML out of cmark_render_xml looks like this
//<?xml version="1.0" encoding="UTF-8"?>
//<!DOCTYPE document SYSTEM "CommonMark.dtd">
//<document xmlns="http://commonmark.org/xml/1.0">
//  <paragraph>
//    <text>Hello </text>
//    <emph>
//      <text>world</text>
//    </emph>
//  </paragraph>
//</document>
static std::string clean_cmark_xml(const std::string & input) {
    std::string result = input;
    result = std::regex_replace(result, std::regex(R"(<\?xml version="1.0" encoding="UTF-8"\?>\n)"),"");
    result = std::regex_replace(result, std::regex(R"(<!DOCTYPE document SYSTEM "CommonMark.dtd">\n)"),"");
    result = std::regex_replace(result, std::regex(R"( xmlns="http://commonmark.org/xml/1.0")"),"");
//    result = std::regex_replace(result, std::regex(R"(\n *)"), "");
    return result;
}

//TODO: Plumb the line-number etc through this
static void check_debug_xml_matches(std::string str, std::string needle) {
    mdx::cmark_node_ptr document_raw(cmark_parse_document(str.c_str(), str.size(), CMARK_OPT_DEFAULT));
    char * p = cmark_render_xml(document_raw.get(), CMARK_OPT_DEFAULT);
    std::string orig_xml = clean_cmark_xml(p);
    free(p);

    auto doc = mdx::fromCMark(document_raw.get());
    assertThat(doc->asDebugXML(), is(orig_xml));

    assertThat(orig_xml, contains(needle));


    mdx::cmark_node_ptr copy = std::move(doc->toCMark()[0]);

     p = cmark_render_xml(copy.get(), CMARK_OPT_DEFAULT);
     std::string copy_xml = clean_cmark_xml(p);
     free(p);

    assertThat(copy_xml, is(orig_xml));

    //Clone should work too...
    assertThat(doc->clone()->asDebugXML(), is(orig_xml));
}
