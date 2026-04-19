/* date = January 31st 2026 4:00 pm */

#ifndef _LONG_MARK_H
#define _LONG_MARK_H

typedef enum LM_NodeType LM_NodeType;
enum LM_NodeType
{
    LM_NodeType_None,
    
    // Block 
    LM_NodeType_Document,
    LM_NodeType_OrList,
    LM_NodeType_UnList,
    LM_NodeType_Quote,
    LM_NodeType_Heading,
    LM_NodeType_Divider,
    LM_NodeType_TextBlock,
    LM_NodeType_CodeBlock,
    
    // Inline 
    LM_NodeType_Text,
    LM_NodeType_Soft_Break,
    LM_NodeType_Line_Break,
    LM_NodeType_Code,
    LM_NodeType_HTML_Inline,
    LM_NodeType_Custom_Inline,
    LM_NodeType_Emph,
    LM_NodeType_Strong,
    LM_NodeType_Link,
    LM_NodeType_Image,
    
    // Error status
	None,
    
	// Block 
	Document,
	Block_Quote,
	List,
	Item,
	Code_Block,
	HTML_Block,
	Custom_Block,
	Paragraph,
	Heading,
	Thematic_Break,
    
	// Inline 
	Text,
	Soft_Break,
	Line_Break,
	Code,
	HTML_Inline,
	Custom_Inline,
	Emph,
	Strong,
	Link,
	Image,
};

typedef struct LM_Node LM_Node;
struct LM_Node
{
    LM_Node* next;
    LM_Node* prev;
    LM_Node* parent;
    LM_Node* first;
    LM_Node* last;
    LM_Node* tag;
    
    LM_NodeType type;
    i32 value;
    String str;
};

#endif //_LONG_MARK_H
