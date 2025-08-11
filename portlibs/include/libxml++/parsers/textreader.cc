#include <libxml++/parsers/textreader.h>
#include <libxml++/exceptions/internal_error.h>

#include <libxml/xmlreader.h>

namespace xmlpp
{
  
  struct TextReader::PropertyReader
  {
    TextReader & owner_;
    PropertyReader(TextReader & owner)
    : owner_(owner)
    {}

    int Int(int value);
    bool Bool(int value);
    char Char(int value);
    std::string String(xmlChar * value, bool free = false);
    std::string String(xmlChar const * value);
  };

TextReader::TextReader(
    struct _xmlTextReader* cobj)
    : propertyreader(new PropertyReader(*this)), impl_( cobj )
{
	
}

TextReader::TextReader(
	const unsigned char* data, 
	size_type size,
	const std::string& uri)
	: propertyreader(new PropertyReader(*this)), 
	  impl_( xmlReaderForMemory ((const char*)data, size, uri.c_str(), NULL, 0) )
{
  if( ! impl_ )
  {
    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
    throw internal_error("Cannot instantiate underlying libxml2 structure");
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
  }
}

TextReader::TextReader(
    const std::string& URI)
  : propertyreader(new PropertyReader(*this)), impl_( xmlNewTextReaderFilename(URI.c_str()) )
{
  if( ! impl_ )
  {
    #ifdef LIBXMLCPP_EXCEPTIONS_ENABLED
    throw internal_error("Cannot instantiate underlying libxml2 structure");
    #endif //LIBXMLCPP_EXCEPTIONS_ENABLED
  }
}

TextReader::~TextReader()
{
  xmlFreeTextReader(impl_);
}

bool TextReader::read()
{
  return propertyreader->Bool(
      xmlTextReaderRead(impl_));
}

std::string TextReader::read_inner_xml()
{
  return propertyreader->String(
      xmlTextReaderReadInnerXml(impl_), true);
}

std::string TextReader::read_outer_xml()
{
  return propertyreader->String(
      xmlTextReaderReadOuterXml(impl_), true);
}

std::string TextReader::read_string()
{
  return propertyreader->String(
      xmlTextReaderReadString(impl_), true);
}

bool TextReader::read_attribute_value()
{
  return propertyreader->Bool(
      xmlTextReaderReadAttributeValue(impl_));
}

int TextReader::get_attribute_count() const
{
  return propertyreader->Int(
      xmlTextReaderAttributeCount(impl_));
}

std::string TextReader::get_base_uri() const
{
  return propertyreader->String(
      xmlTextReaderBaseUri(impl_));
}

int TextReader::get_depth() const
{
  return propertyreader->Int(
      xmlTextReaderDepth(impl_));
}

bool TextReader::has_attributes() const
{
  return propertyreader->Bool(
      xmlTextReaderHasAttributes(impl_));
}

bool TextReader::has_value() const
{
  return propertyreader->Bool(
      xmlTextReaderHasValue(impl_));
}

bool TextReader::is_default() const
{
  return propertyreader->Bool(
      xmlTextReaderIsDefault(impl_));
}

bool TextReader::is_empty_element() const
{
  return propertyreader->Bool(
      xmlTextReaderIsEmptyElement(impl_));
}

std::string TextReader::get_local_name() const
{
  return propertyreader->String(
      xmlTextReaderLocalName(impl_), true);
}

std::string TextReader::get_name() const
{
  return propertyreader->String(
      xmlTextReaderName(impl_), true);
}

std::string TextReader::get_namespace_uri() const
{
  return propertyreader->String(
      xmlTextReaderNamespaceUri(impl_), true);
}

TextReader::xmlNodeType TextReader::get_node_type() const
{
  int result = xmlTextReaderNodeType(impl_);
  if(result == -1)
    check_for_exceptions();
  return (xmlNodeType)result;
}

std::string TextReader::get_prefix() const
{
  return propertyreader->String(
      xmlTextReaderPrefix(impl_), true);
}

char TextReader::get_quote_char() const
{
  return propertyreader->Char(
      xmlTextReaderQuoteChar(impl_));
}

std::string TextReader::get_value() const
{
  return propertyreader->String(
      xmlTextReaderValue(impl_), true);
}

std::string TextReader::get_xml_lang() const
{
  return propertyreader->String(
      xmlTextReaderXmlLang(impl_));
}

TextReader::xmlReadState TextReader::get_read_state() const
{
  int result = xmlTextReaderReadState(impl_);
  if(result == -1)
    check_for_exceptions();
  return (xmlReadState)result;
}

void TextReader::close()
{
  if(xmlTextReaderClose(impl_) == -1)
    check_for_exceptions();
}

std::string TextReader::get_attribute(int number) const
{
  return propertyreader->String(
      xmlTextReaderGetAttributeNo(impl_, number), true);
}

std::string TextReader::get_attribute(
    const std::string& name) const
{
  return propertyreader->String(
      xmlTextReaderGetAttribute(impl_, (const xmlChar *)name.c_str()), true);
}

std::string TextReader::get_attribute(
    const std::string& localName,
    const std::string& namespaceURI) const
{
  return propertyreader->String(
      xmlTextReaderGetAttributeNs(impl_, (const xmlChar *)localName.c_str(), (const xmlChar *)namespaceURI.c_str()), true);
}

std::string TextReader::lookup_namespace(
    const std::string& prefix) const
{
  return propertyreader->String(
      xmlTextReaderLookupNamespace(impl_, (const xmlChar *)prefix.c_str()), true);
}

bool TextReader::move_to_attribute(int number)
{
  return propertyreader->Bool(
      xmlTextReaderMoveToAttributeNo(impl_, number));
}

bool TextReader::move_to_attribute(
    const std::string& name)
{
  return propertyreader->Bool(
      xmlTextReaderMoveToAttribute(impl_, (const xmlChar *)name.c_str()));
}

bool TextReader::move_to_attribute(
    const std::string& localName,
    const std::string& namespaceURI)
{
  return propertyreader->Bool(
      xmlTextReaderMoveToAttributeNs(impl_, (const xmlChar *)localName.c_str(), (const xmlChar *)namespaceURI.c_str()));
}

bool TextReader::move_to_first_attribute()
{
  return propertyreader->Bool(
      xmlTextReaderMoveToFirstAttribute(impl_));
}

bool TextReader::move_to_next_attribute()
{
  return propertyreader->Bool(
      xmlTextReaderMoveToNextAttribute(impl_));
}

bool TextReader::move_to_element()
{
  return propertyreader->Bool(
      xmlTextReaderMoveToElement(impl_));
}

bool TextReader::get_normalization() const
{
  return propertyreader->Bool(
      xmlTextReaderNormalization(impl_));
}

bool TextReader::get_parser_property(
    ParserProperties property) const
{
  return propertyreader->Bool(
      xmlTextReaderGetParserProp(impl_, (int)property));
}

void TextReader::set_parser_property(
    ParserProperties property,
    bool value)
{
  if(xmlTextReaderSetParserProp(impl_, (int)property, value?1:0))
    check_for_exceptions();
}

Node* TextReader::get_current_node()
{
  xmlNodePtr node = xmlTextReaderCurrentNode(impl_);
  if(node)
    return static_cast<Node*>(node->_private);
    
  check_for_exceptions();
  return 0;
}

const Node* TextReader::get_current_node() const
{
  xmlNodePtr node = xmlTextReaderCurrentNode(impl_);
  if(node)
    return static_cast<Node*>(node->_private);

  check_for_exceptions();
  return 0;
}

/*
TODO: add a private constructor to Document.
Document* TextReader::CurrentDocument()
{
  xmlDocPtr doc = xmlTextReaderCurrentDoc(impl_);
  if(doc)
    return new Document(doc);
}
*/

Node* TextReader::expand()
{
  xmlNodePtr node = xmlTextReaderExpand(impl_);
  if(node)
    return static_cast<Node*>(node->_private);
    
  check_for_exceptions();
  return 0;
}

bool TextReader::next()
{
  return propertyreader->Bool(
      xmlTextReaderNext(impl_));
}

bool TextReader::is_valid() const
{
  return propertyreader->Bool(
      xmlTextReaderIsValid(impl_));
}




int TextReader::PropertyReader::Int(
    int value)
{
  if(value == -1)
    owner_.check_for_exceptions();
  return value;
}

bool TextReader::PropertyReader::Bool(
    int value)
{
  if(value == -1) {
    owner_.check_for_exceptions();
    value = 0;
  }
    
  return value;
}

char TextReader::PropertyReader::Char(
    int value)
{
  owner_.check_for_exceptions();
  return value;
}

std::string TextReader::PropertyReader::String(
    xmlChar * value,
    bool free)
{
  owner_.check_for_exceptions();
  
  if(value == (xmlChar *)0)
    return std::string();
    
  std::string result = (char *)value;

  if(free)
    xmlFree(value);

  return result;
}

std::string TextReader::PropertyReader::String(
    xmlChar const * value)
{
  owner_.check_for_exceptions();

  if(value == (xmlChar *)0)
    return std::string();

  return (const char *)value;
}

void TextReader::check_for_exceptions() const
{
  //TODO: Shouldn't we do something here? murrayc.
}

} // namespace xmlpp
