#pragma once

#include "wrap32lib.h"

#include "StringParser.h"

#include <map>
#include <sstream>
#include <vector>
#include <functional>

enum jsonType {
	typeString,
	typeInt,
	typeDouble,
	typeBool,
	typeNull,
	typeObject,
	typeArray,
};

namespace json {

	class jsonBase
	{
	public:
		jsonBase() {}
		virtual ~jsonBase() {}
		virtual void GetJSON(std::wstringstream& wss) const = 0;

		virtual jsonBase* Clone() const = 0;

		virtual jsonType GetType() const = 0;
	};

	class jsonString : public jsonBase
	{
	public:
		jsonString(LPCWSTR value) : m_value(value) {}
		jsonString(const std::wstring& value) : m_value(value.c_str()) {}

		void GetJSON(std::wstringstream& wss) const override {
			wss << "\"" << m_value << "\"";
		}

		const wchar_t* GetValue() { return m_value.c_str();  }

		jsonString& operator=(const jsonString& rhs) {
			m_value = rhs.m_value;
			return *this;
		}

		jsonBase* Clone() const override {
			return new jsonString(*this);
		}

		jsonType GetType() const { return typeString;  }
	protected:
		std::wstring m_value;
	};

	class jsonInt : public jsonBase
	{
	public:
		jsonInt(int value) : m_value(value) {}

		void GetJSON(std::wstringstream& wss) const override {
			wss << m_value;
		}

		int GetValue() { return m_value; }

		jsonInt& operator=(const jsonInt& rhs) {
			m_value = rhs.m_value;
			return *this;
		}

		jsonBase* Clone() const override {
			return new jsonInt(*this);
		}

		jsonType GetType() const { return typeInt; }

	protected:
		int m_value;
	};

	class jsonDouble : public jsonBase
	{
	public:
		jsonDouble(double value) : m_value(value) {}

		void GetJSON(std::wstringstream& wss) const override {
			wss << m_value;
		}

		double GetValue() { return m_value; }

		jsonDouble& operator=(const jsonDouble& rhs) {
			m_value = rhs.m_value;
			return *this;
		}

		jsonBase* Clone() const override {
			return new jsonDouble(*this);
		}

		jsonType GetType() const { return typeDouble; }

	protected:
		double m_value;
	};

	class jsonNull : public jsonBase
	{
	public:
		void GetJSON(std::wstringstream& wss) const override {
			wss << "null";
		}

		jsonBase* Clone() const override {
			return new jsonNull;
		}

		jsonType GetType() const { return typeNull; }
	};

	class jsonBool : public jsonBase
	{
	public:
		jsonBool(bool value) : m_value(value) {}

		void GetJSON(std::wstringstream& wss) const override {
			wss << (m_value ? L"true" : L"false");
		}

		bool GetValue() { return m_value; }

		jsonBool& operator=(const jsonBool& rhs) {
			m_value = rhs.m_value;
			return *this;
		}

		jsonBase* Clone() const override {
			return new jsonBool(*this);
		}

		jsonType GetType() const { return typeBool; }

	protected:
		bool m_value;
	};

	class jsonArray;

	class jsonCollection : public jsonBase
	{
	public:
		virtual void Span(std::function<void(jsonBase*)> f) = 0;
	};

	class jsonObject : public jsonCollection
	{
	public:
		jsonObject() {}
		jsonObject(const jsonObject& rhs) { *this = rhs; }
		~jsonObject() {
			Clear();
		}

		void Clear() {
			for (auto p : m_properties)
				delete p.second;
			m_properties.clear();
		}

		bool IsEmpty() const {
			return m_properties.empty();
		}

		jsonObject& operator=(const jsonObject& rhs) {
			for (auto p : m_properties)
				delete p.second;
			m_properties.clear();

			for (auto p : rhs.m_properties) {
				m_properties[p.first] = p.second->Clone();
			}

			return *this;
		}

		void SetProperty(LPCWSTR szName, LPCWSTR value) {
			auto it = m_properties.find(szName);
			if (it != m_properties.end()) {
				delete it->second;
				it->second = new jsonString(value);
			}
			else
				m_properties[szName] = new jsonString(value);
		}
		void SetProperty(LPCWSTR szName, const std::wstring& value)	{	SetProperty(szName, value.c_str()); }
		void SetProperty(LPCWSTR szName, const std::string& value)	{	SetProperty(szName, UnicodeMultibyte(value.c_str())); }

		void SetProperty(LPCWSTR szName, int value) {
			auto it = m_properties.find(szName);
			if (it != m_properties.end()) {
				delete it->second;
				it->second = new jsonInt(value);
			}
			else
				m_properties[szName] = new jsonInt(value);
		}

		void SetProperty(LPCWSTR szName, double value) {
			auto it = m_properties.find(szName);
			if (it != m_properties.end()) {
				delete it->second;
				it->second = new jsonDouble(value);
			}
			else
				m_properties[szName] = new jsonDouble(value);
		}

		void SetProperty(LPCWSTR szName, bool value) {
			auto it = m_properties.find(szName);
			if (it != m_properties.end()) {
				delete it->second;
				it->second = new jsonBool(value);
			}
			else
				m_properties[szName] = new jsonBool(value);
		}

		// Don't use these - Legacy
		void SetProperty(LPCWSTR szName, const jsonArray& array);
		void SetProperty(LPCWSTR szName, const jsonObject& object);

		// Use these
		jsonArray* AddArrayProperty(LPCWSTR szName);
		jsonObject* AddObjectProperty(LPCWSTR szName);

		void GetJSON(std::wstringstream& wss) const override {
			wss << L"{";
			bool bFirst = true;
			for (auto property : m_properties) {
				if (bFirst)
					bFirst = false;
				else
					wss << L",";
				wss << L"\"" << property.first << L"\":";
				property.second->GetJSON(wss);
			}
			wss << L"}";
		}

		jsonBase* Clone() const override {
			return new jsonObject(*this);
		}

		bool Parse(const wchar_t* sz, int& line, std::wstring& err) {
			WStringParser wsp(sz);
			return Parse(wsp, line, err);
		}

		bool Parse(WStringParser& wsp, int& line, std::wstring& err);

		const std::map<std::wstring, jsonBase*>& GetProperties() const {
			return m_properties;
		}

		jsonBase* GetProperty(LPCWSTR sz) const {
			auto it = m_properties.find(sz);
			return (it == m_properties.end()) ? NULL : it->second;
		}

		jsonInt* GetIntProperty(LPCWSTR sz) const {
			auto it = m_properties.find(sz);
			if (it == m_properties.end())
				return NULL;
			return (it->second->GetType() != typeInt) ? NULL : (jsonInt*)it->second;
		}

		jsonDouble* GetDoubleProperty(LPCWSTR sz) const {
			auto it = m_properties.find(sz);
			if (it == m_properties.end())
				return NULL;
			return (it->second->GetType() != typeDouble) ? NULL : (jsonDouble*)it->second;
		}

		jsonBool* GetBoolProperty(LPCWSTR sz) const {
			auto it = m_properties.find(sz);
			if (it == m_properties.end())
				return NULL;
			return (it->second->GetType() != typeBool) ? NULL : (jsonBool*)it->second;
		}

		jsonString* GetStringProperty(LPCWSTR sz) const {
			auto it = m_properties.find(sz);
			if (it == m_properties.end())
				return NULL;
			return (it->second->GetType() != typeString) ? NULL : (jsonString *)it->second;
		}

		jsonObject* GetObjectProperty(LPCWSTR sz) const {
			auto it = m_properties.find(sz);
			if (it == m_properties.end())
				return NULL;
			return (it->second->GetType() != typeObject) ? NULL : (jsonObject*)it->second;
		}

		jsonArray* GetArrayProperty(LPCWSTR sz) const {
			auto it = m_properties.find(sz);
			if (it == m_properties.end())
				return NULL;
			return (it->second->GetType() != typeArray) ? NULL : (jsonArray*)it->second;
		}

		jsonType GetType() const { return typeObject; }

		void Span(std::function<void(jsonBase*)> f) override;

		json::jsonBase* GetElementByPath(LPCWSTR path) const;

	protected:
		std::map<std::wstring, jsonBase*> m_properties;
	};

	class jsonArray : public jsonCollection
	{
	public:
		jsonArray() {}
		jsonArray(const jsonArray& rhs) { *this = rhs; }
		~jsonArray() {
			for (auto p : m_array)
				delete p;
		}

		jsonArray& operator=(const jsonArray& rhs) {
			for (auto p : m_array)
				delete p;
			m_array.clear();

			for (auto p : rhs.m_array) {
				m_array.push_back(p->Clone());
			}

			return *this;
		}

		jsonObject* AddNewObject() {
			jsonObject* p = new jsonObject;
			m_array.push_back(p);
			return p;
		}

		jsonArray* AddNewArray() {
			jsonArray* p = new jsonArray;
			m_array.push_back(p);
			return p;
		}

		void Add(bool value)	{	m_array.push_back(new jsonBool(value));		}
		void Add(int value)		{	m_array.push_back(new jsonInt(value));		}
		void Add(LPCWSTR value) {	m_array.push_back(new jsonString(value));	}
		void Add(const std::wstring& value) 
								{	m_array.push_back(new jsonString(value));	}

		void GetJSON(std::wstringstream& wss) const override {
			wss << L"[";
			bool bFirst = true;
			for (auto p : m_array) {
				if (bFirst)
					bFirst = false;
				else
					wss << L",";
				p->GetJSON(wss);
			}
			wss << L"]";
		}

		jsonBase* Clone() const override {
			return new jsonArray(*this);
		}

		jsonType GetType() const { return typeArray; }

		// Array access
		size_t Size() const { return m_array.size();  }

		jsonType TypeAt(size_t index) { return m_array.at(index)->GetType(); }
		jsonBase* GetAt(size_t index) { return m_array.at(index); }
		jsonObject* GetObjectAt(size_t index) const {
			jsonBase* p = m_array.at(index);
			return (!p || (p->GetType() != typeObject)) ? NULL : (jsonObject*)p;
		}
		jsonInt* GetIntAt(size_t index) const {
			jsonBase* p = m_array.at(index);
			return (!p || (p->GetType() != typeInt)) ? NULL : (jsonInt*)p;
		}

		jsonDouble* GetDoubleAt(size_t index) const {
			jsonBase* p = m_array.at(index);
			return (!p || (p->GetType() != typeDouble)) ? NULL : (jsonDouble*)p;
		}

		jsonString* GetStringAt(size_t index) const {
			jsonBase* p = m_array.at(index);
			return (!p || (p->GetType() != typeString)) ? NULL : (jsonString*)p;
		}

		// Loading from string
		bool Parse(const wchar_t* sz, int& line, std::wstring& err) {
			WStringParser wsp(sz);
			return Parse(wsp, line, err);
		}

		bool Parse(WStringParser& wsp, int& line, std::wstring& err);

		void Span(std::function<void(jsonBase*)> f) override;
		json::jsonBase* GetElementByPath(LPCWSTR path) const;

		bool GetAsInts(std::vector<int>& items) {
			size_t size = Size();
			for (size_t i = 0; i < size; i++) {
				json::jsonInt* ji = GetIntAt(i);
				if (!ji) return false;
				items.push_back(ji->GetValue());
			}
			return true;
		}

		bool GetAsStrings(std::vector<std::wstring>& items) {
			size_t size = Size();
			for (size_t i = 0; i < size; i++) {
				json::jsonString* ji = GetStringAt(i);
				if (!ji) return false;
				items.push_back(ji->GetValue());
			}
			return true;
		}
	protected:
		std::vector<jsonBase*> m_array;
	};

	class jsonBuilder {
	public:
		jsonBuilder(jsonType type):  m_type(type), m_started(false), m_indent(0) {}
		jsonBuilder(jsonType type, const jsonBuilder& parent) : m_type(type), m_started(false), m_indent(parent.m_indent) {
			if (!parent.m_started) {
				m_indent++;
			}
		}
		~jsonBuilder() {	Finish();		}

		void Add(LPCWSTR name, LPCWSTR value) {
			CheckStarted();
			m_wss << "\"" << name << "\" : \"" << value << "\"";
		}
		void Add(LPCWSTR name, const std::wstring& value) {	Add(name, value.c_str());	}

		void Add(LPCWSTR name, DWORD value) {
			CheckStarted();
			m_wss << "\"" << name << "\" : " << value;
		}

		void Add(LPCWSTR name, bool value) {
			CheckStarted();
			m_wss << "\"" << name << "\" : " << (value ? "true" : "false");
		}

		void Add(jsonBuilder& jb) {
			if (m_type != typeArray) {
				return;
			}
			jb.Finish();	// Make sure it's finished
			CheckStarted();
			m_wss << jb.m_wss.str();
		}

		void Add(LPCWSTR name, jsonBuilder& jb) {
			if (m_type != typeObject) {
				return;
			}
			jb.Finish();	// Make sure it's finished
			CheckStarted();

			m_wss << "\"" << name << "\" : " << jb.m_wss.str();
		}

		void Add(int value) {
			if (m_type != typeArray)
				return;

			CheckStarted();
			m_wss << value;
		}

		void Add(LPCWSTR value) {
			if (m_type != typeArray)
				return;

			CheckStarted();
			m_wss << L"\"" << value << L"\"";
		}

		void Finish() {
			if (!m_started)
				CheckStarted();

			if (m_type == typeArray) {
				m_wss << L"\n";
				m_indent--;
				Indent();
				m_wss << L"]";
			}
			else if (m_type == typeObject) {
				m_wss << L"\n";
				m_indent--;
				Indent();
				m_wss << L"}";
			}
//			m_started = false;
		}

		std::wstring str() {
			Finish();	// make sure it's finished
			return m_wss.str();
		}

	protected:
		void CheckStarted() {
			if (!m_started) {
				if (m_type == typeArray) {
					m_wss << L"[\n";
					m_indent++;
				}
				else if (m_type == typeObject) {
					m_wss << L"{\n";
					m_indent++;
				}

				m_started = true;
			}
			else
				m_wss << ",\n";

			Indent();
		}

		void Indent() {
			for (int i = 0; i < m_indent; i++) {
				m_wss << L"  ";
			}
		}

	protected:
		std::wstringstream m_wss;
		bool m_started;
		jsonType m_type;
		int m_indent;
	};
}