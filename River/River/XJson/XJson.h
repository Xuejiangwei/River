#pragma once
#include <vector>
#include <memory>
#include <string>

enum class JsonType
{
	None,
	String,
	Number,
	Object,
	Array,
};

class XJson
{
private:
	class XJsonList
	{
		friend class XJson;
	public:
		XJsonList() {}

		~XJsonList() {}

		void Clear() { m_Childs.clear(); }

		bool Empty() { return m_Childs.empty(); }
		
		unsigned long long Size() { return m_Childs.size(); }
		
		void Add(std::unique_ptr<XJson>& node) { m_Childs.push_back(std::move(node)); }

		XJson* operator[](unsigned long long idx) { return m_Childs[idx].get(); }
		
		bool Remove(XJson* node)
		{
			for (auto iter = m_Childs.begin(); iter != m_Childs.end(); iter++)
			{
				if (iter->get() == node)
				{
					m_Childs.erase(iter);
					return true;
				}
			}

			return false;
		}

	private:
		std::vector<std::unique_ptr<XJson>> m_Childs;
	};

	class JsonBuffer
	{
		friend class XJson;
	public:
		JsonBuffer() : m_Root(nullptr), m_Offset(0), m_Data(nullptr), m_Size(0)
		{}
		
		~JsonBuffer() {}

		void StartRead();
		
		void StartWrite();

	private:
		XJson* m_Root;

		char* m_Data;
		unsigned long long m_Size;
		unsigned long long m_Offset;

		std::string m_ReadBuffer;
		std::string m_WriteBuffer;
	};

	class JsonNodeData
	{
		friend class XJson;
	public:
		enum class DataType
		{
			None = 0,
			Bool,
			Int32,
			UInt32,
			Int64,
			UInt64,
			Float,
			Double,
			String
		};

		JsonNodeData(DataType type = DataType::None);

		~JsonNodeData();

		void Clear();
		
		void ToString();
		
		void SetType(DataType type);

	private:
		union
		{
			bool BoolValue;
			int IntValue;
			unsigned int UIntValue;
			int64 Int64Value;
			unsigned long long UInt64Value;
			float FloatValue;
			double DoubleValue;
		} m_Value;

		DataType m_Type;
		std::string m_Content;
	};

public:
	XJson(JsonType type = JsonType::None);

	~XJson();

	void operator=(bool val);
	
	void operator=(int val);
	
	void operator=(unsigned int val);
	
	void operator=(int64 val);
	
	void operator=(unsigned long long val);
	
	void operator=(float val);

	void operator=(double val);
	
	void operator=(const char* val);
	
	void operator=(const std::string& val);

	XJson& operator[](int idx) { return SetArray(idx); }

	XJson& operator[](unsigned long long idx) { return SetArray(idx); }

	XJson& operator[](const char* str) { return SetObject(str); }

	XJson& operator[](const std::string& str) { return SetObject(str.c_str()); }

	unsigned long long Size() { return m_JsonValue ? m_JsonValue->Size() : 0; }

	bool Empty() { return m_JsonValue ? m_JsonValue->Empty() : true; }

	std::unique_ptr<XJson> CreateNode(char code);

	bool MakeReadContext();

	void AddNode(std::unique_ptr<XJson>& node);

	void Clear();

	void TrimSpace();

	char GetCharCode();

	char GetChar();

	char CheckCode(char charCode);

	unsigned long long SearchCode(char code);

	JsonType CodeToType(char code);
	
	const char* NodeDataString();

	const std::string& NodeDataStringRef();

	const char* KeyNodeName();
	
	const char* Data();

	const std::string& Encode();

	bool Decode(const std::string& buffer);

	bool DecodeFromFile(const std::string& filePath);

	int StringToInt32();
	long long StringToInt64();
	double StringToDouble();

private:
	void Read(std::shared_ptr<JsonBuffer> context, bool isRoot = false);

	void ReadNumber();
	
	void ReadString();
	
	void ReadObject();
	
	void ReadArray();

	void Write(std::shared_ptr<JsonBuffer> context, bool isRoot = false);

	void WriteNumber();
	
	void WriteString();
	
	void WriteObject();
	
	void WriteArray();

	XJson& SetArray(unsigned long long idx);

	XJson& SetObject(const char* str);

private:
	JsonType m_Type;
	unsigned long long m_ReadIndex;
	unsigned long long m_Length;

	std::unique_ptr<XJson> m_KeyNameNode;
	std::unique_ptr<XJsonList> m_JsonValue;
	std::unique_ptr<JsonNodeData> m_NodeData;

	std::shared_ptr<JsonBuffer> m_DecodeContext;
	std::shared_ptr<JsonBuffer> m_Encodecontext;
};
