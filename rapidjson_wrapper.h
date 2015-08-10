/****************************************************************************

Creator: liyangyao@gmail.com
Date: 2015/4/20

****************************************************************************/

#ifndef RAPIDJSON_WRAPPER_H
#define RAPIDJSON_WRAPPER_H

#include <string>
#include <QVariant>
#include <QVariantList>
#include <QDebug>
#include <rapidjson/reader.h>
#include <rapidjson/writer.h>
#include <rapidjson/error/en.h>

#pragma execution_character_set("utf-8")

namespace json
{
namespace inner
{
class JsonObject
{
public:
    explicit JsonObject(JsonObject* parent):
        m_parent(parent)
    {

    }

    virtual ~JsonObject()
    {

    }

    void putKey(const QString &k)
    {
        m_key = k;
    }

    QString& key()
    {
        return m_key;
    }

    JsonObject* parent()
    {
        return m_parent;
    }

    virtual void putValue(const QVariant &)
    {

    }

private:
    QString m_key;
    JsonObject* m_parent;
    Q_DISABLE_COPY(JsonObject)
};

class MapObject:public JsonObject
{
public:
    explicit MapObject(JsonObject* parent):
        JsonObject(parent)
    {

    }


    virtual void putValue(const QVariant &v)
    {
        m_map.insert(key(), v);
    }

    QVariantMap map()
    {
        return m_map;
    }

private:
    QVariantMap m_map;
    Q_DISABLE_COPY(MapObject)
};

class ListObject:public JsonObject
{
public:
    explicit ListObject(JsonObject* parent):
        JsonObject(parent)
    {

    }

    virtual void putValue(const QVariant &v)
    {
        m_list.push_back(v);
    }

    QVariantList list()
    {
        return m_list;
    }

private:
    QVariantList m_list;
    Q_DISABLE_COPY(ListObject)
};

class MessageHandler: public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, MessageHandler>
{
public:
    explicit MessageHandler():
        m_currentObject(nullptr)
    {
    }

    ~MessageHandler()
    {
        while(m_currentObject)
        {
            JsonObject* object = m_currentObject;
            m_currentObject = object->parent();
            delete object;
        }
    }

    QVariantMap result()
    {
        return m_map;
    }

    bool StartObject()
    {
        m_currentObject = new MapObject(m_currentObject);
        return true;
    }

    bool EndObject(rapidjson::SizeType)
    {
        if (m_currentObject)
        {
            MapObject* object = (MapObject *)m_currentObject;
            m_currentObject = m_currentObject->parent();
            if (!m_currentObject)
            {
                m_map = object->map();
            }
            else{
                m_currentObject->putValue(object->map());
            }
            delete object;
        }
        return true;
    }

    bool StartArray()
    {
        m_currentObject = new ListObject(m_currentObject);
        return true;
    }

    bool EndArray(rapidjson::SizeType )
    {
        ListObject* object =  (ListObject *)m_currentObject;
        m_currentObject = m_currentObject->parent();
        m_currentObject->putValue(object->list());
        delete object;
        return true;
    }

    bool Key(const char* str, rapidjson::SizeType length, bool)
    {
        QString key = QString(QByteArray::fromRawData(str, length));
        m_currentObject->putKey(key);
        return true;
    }

    bool String(const char* str, rapidjson::SizeType length, bool)
    {        
        m_currentObject->putValue(QString(QByteArray::fromRawData(str, length)));
        return true;
    }

    bool Int(int i)
    {
        m_currentObject->putValue(i);
        return true;
    }

    bool Int64(int64_t i)
    {
       m_currentObject->putValue(i);
        return true;
    }

    bool Uint(unsigned u)
    {
        m_currentObject->putValue(u);
        return true;
    }

    bool Uint64(uint64_t u)
    {
        m_currentObject->putValue(u);
        return true;
    }

     bool Double(double d)
     {
         m_currentObject->putValue(d);
         return true;
     }

    bool Default()
    {
        m_currentObject->putValue(QVariant());
        return true;
    }

    bool Bool(bool b)
    {
        m_currentObject->putValue(b);
        return true;
    }


private:
    JsonObject* m_currentObject;
    QVariantMap m_map;
    Q_DISABLE_COPY(MessageHandler)
};
}

QVariantMap parse(const QByteArray &json, bool &success)
{
    rapidjson::Reader reader;
    inner::MessageHandler handler;
    rapidjson::StringStream ss(json.constData());
    success = reader.Parse(ss, handler);
    if (!success)
    {
        rapidjson::ParseErrorCode e = reader.GetParseErrorCode();
        size_t o = reader.GetErrorOffset();
        qDebug()<<"error offset:"<<o;
        qDebug()<<"Error: "<<rapidjson::GetParseError_En(e);
    }
    return handler.result();
}

QVariantMap parse(const QByteArray &json)
{
    bool success;
    return parse(json, success);
}

//添加支持QString,QVariant
class Serialize
{
public:
    explicit Serialize():
        m_buffer(),
        m_writer(m_buffer)
    {

    }

    void startArray(const char* s=nullptr)
    {
        if (s)
        {
            m_writer.String(s);
        }
        m_writer.StartArray();
    }

    void endArray()
    {
        m_writer.EndArray();
    }

    void startObject(const char* s=nullptr)
    {
        if (s)
        {
            m_writer.String(s);
        }
        m_writer.StartObject();
    }

    void endObject()
    {
        m_writer.EndObject();
    }

    const char* getString() const
    {
        return m_buffer.GetString();
    }

    Serialize& operator <<(const char* s)
    {
        m_writer.String(s);
        return *this;
    }

    Serialize& operator <<(const QString& s)
    {
        string(s);
        return *this;
    }

    Serialize& operator <<(int i)
    {
        m_writer.Int(i);
        return *this;
    }

    Serialize& operator <<(unsigned i)
    {
        m_writer.Uint(i);
        return *this;
    }

    Serialize& operator <<(int64_t i64)
    {
        m_writer.Int64(i64);
        return *this;
    }

    Serialize& operator <<(uint64_t u64)
    {
        m_writer.Uint64(u64);
        return *this;
    }

    Serialize& operator <<(double d)
    {
        m_writer.Double(d);
        return *this;
    }

    Serialize& operator <<(bool b)
    {
        m_writer.Bool(b);
        return *this;
    }

    Serialize& operator <<(const QVariant& v)
    {
        variant(v);
        return *this;
    }

    Serialize& operator <<(std::nullptr_t)
    {
        m_writer.Null();
        return *this;
    }

    const char* GetString()
    {
        return m_buffer.GetString();
    }

private:
    rapidjson::Writer<rapidjson::StringBuffer> m_writer;
    rapidjson::StringBuffer m_buffer;
    Serialize& string(const QString& s)
    {
        QByteArray data = s.toUtf8();
        m_writer.String(data.constData(), data.length());
        return *this;
    }

    Serialize& variant(const QVariant& v)
    {
        if (v.isNull())
        {
            m_writer.Null();
            return *this;
        }
        switch(v.type())
        {
        case QVariant::String:
            string(v.toString());
            break;
        case QVariant::Int:
            m_writer.Int(v.toInt());
            break;
        case QVariant::UInt:
            m_writer.Uint(v.toUInt());
            break;
        case QVariant::LongLong:
            m_writer.Int64(v.toLongLong());
            break;
        case QVariant::ULongLong:
           m_writer.Uint64(v.toULongLong());
            break;
        case QVariant::Bool:
            m_writer.Bool(v.toBool());
            break;
        case QVariant::Double:
            m_writer.Double(v.toDouble());
            break;
        default:
            string(v.toString());
        }
        return *this;

    }
    Q_DISABLE_COPY(Serialize)
};
}

#endif // RAPIDJSON_WRAPPER_H
