# Qt-rapidjson

Qt easy use rapidjson(https://github.com/miloyip/rapidjson)

void testParse(const QByteArray &data)
{
    QVariantMap json = json::parse(data);
    Q_ASSERT(json["name"].toString()=="cplusplus");
    Q_ASSERT(json["age"].toInt()==10);
    QVariantMap detail = json["detail"].toMap();
    Q_ASSERT(detail["hasDetail"].toBool());
    Q_ASSERT(qFuzzyIsNull(detail["pi"].toDouble() - 3.14));
    QVariantList array = json["array"].toList();
    Q_ASSERT(array[0].toInt()==1);
    Q_ASSERT(array[1].toString()=="two");
    Q_ASSERT(array[2].isNull());
}

QByteArray testSerialize()
{
    json::Serialize s;
    s.startObject();
    s<<"name"<<"cplusplus";
    s<<"age"<<10;

    s.startObject("detail");
    s<<"hasDetail"<<true;
    s<<"pi"<<3.14;
    s.endObject();

    s.startArray("array");
    s<<1<<"two"<<nullptr;
    s.endArray();

    s.endObject();

    return QByteArray (s.getString());
}
