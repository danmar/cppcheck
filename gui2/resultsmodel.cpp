#include "resultsmodel.h"
#include <QDate>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QTextStream>

ResultsModel::ResultsModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    rootNode = new Node;
}

void ResultsModel::clear()
{
    delete rootNode;
    rootNode = new Node;
    this->reset();
}

void ResultsModel::addresult(const QString &errmsg)
{
    QString file, line, severity, text, id;
    if (parseErrorMessage(errmsg, &file, &line, &severity, &text, &id)) {
        new Node(rootNode,file,line,severity,text,id);
        this->reset();
    }
}

static QString getstr(const QDomElement element, const QString &tagName)
{
    const QDomElement child = element.firstChildElement(tagName);
    return child.isNull() ? QString() : child.text();
}

bool ResultsModel::load(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QDomDocument doc;
    if (!doc.setContent(&file))
        return false;

    const QDomElement rootElement = doc.documentElement();
    if (rootElement.tagName() != "results")
        return false;

    const QDomElement resultsElement = rootElement.firstChildElement("results");
    if (resultsElement.isNull())
        return false;

    delete rootNode;
    rootNode = new Node;

    for (QDomElement element = resultsElement.firstChildElement(); !element.isNull(); element = element.nextSiblingElement()) {
        if (element.tagName() == "result") {
            Node *node = new Node(rootNode,
                                  getstr(element,"file"),
                                  getstr(element,"line"),
                                  getstr(element,"severity"),
                                  getstr(element,"text"),
                                  getstr(element,"id"));
            node->parent = rootNode;
            rootNode->children.append(node);
        }
    }

    this->reset();
    return true;
}

bool ResultsModel::save(const QString &fileName, const QString &projectName) const
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QDomDocument doc;
    QDomElement root = doc.createElement("results");
    doc.appendChild(root);

    QDomElement meta = doc.createElement("meta");
    QDomElement project     = doc.createElement("project");
    QDomElement date        = doc.createElement("date");

    root.appendChild(meta);
    meta.appendChild(project);
    project.appendChild(doc.createTextNode(projectName));
    meta.appendChild(date);
    date.appendChild(doc.createTextNode(QDate::currentDate().toString("yyyy-MM-dd")));

    QDomElement results = doc.createElement("results");
    root.appendChild(results);
    foreach(const Node *node, rootNode->children) {
        QDomElement result = doc.createElement("result");
        QDomElement file     = doc.createElement("file");
        QDomElement line     = doc.createElement("line");
        QDomElement severity = doc.createElement("severity");
        QDomElement text     = doc.createElement("text");
        QDomElement id       = doc.createElement("id");
        results.appendChild(result);

        result.appendChild(file);
        file.appendChild(doc.createTextNode(node->filename));

        result.appendChild(line);
        line.appendChild(doc.createTextNode(node->line));

        result.appendChild(severity);
        severity.appendChild(doc.createTextNode(node->severity));

        result.appendChild(text);
        text.appendChild(doc.createTextNode(node->text));

        result.appendChild(id);
        id.appendChild(doc.createTextNode(node->id));
    }

    QTextStream out(&file);
    doc.save(out, 4);

    return true;
}



QModelIndex ResultsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!rootNode)
        return QModelIndex();
    Node *parentNode = nodeFromIndex(parent);
    return createIndex(row, column, parentNode->children[row]);
}

ResultsModel::Node *ResultsModel::nodeFromIndex(const QModelIndex &index) const
{
    if (index.isValid())
        return static_cast<Node *>(index.internalPointer());
    else
        return rootNode;
}

int ResultsModel::rowCount(const QModelIndex &parent) const
{
    Node *parentNode = nodeFromIndex(parent);
    if (!parentNode)
        return 0;
    return parentNode->children.count();
}

int ResultsModel::columnCount(const QModelIndex & /* parent */) const
{
    return 5;
}

QModelIndex ResultsModel::parent(const QModelIndex &child) const
{
    Node *node = nodeFromIndex(child);
    if (!node)
        return QModelIndex();
    Node *parentNode = node->parent;
    if (!parentNode)
        return QModelIndex();
    Node *grandparentNode = parentNode->parent;
    if (!grandparentNode)
        return QModelIndex();
    int row = grandparentNode->children.indexOf(parentNode);
    return createIndex(row, child.column(), parentNode);
}

QVariant ResultsModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    Node *node = nodeFromIndex(index);
    if (!node)
        return QVariant();

    switch (index.column()) {
    case 0:
        return node->filename;
    case 1:
        return node->line;
    case 2:
        return node->severity;
    case 3:
        return node->text;
    case 4:
        return node->id;
    default:
        return QVariant();
    }
}

QVariant ResultsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Filename");
        case 1:
            return tr("Line");
        case 2:
            return tr("Severity");
        case 3:
            return tr("Text");
        case 4:
            return tr("Id");
        default:
            return QVariant();
        }
    }
    return QVariant();
}

bool ResultsModel::parseErrorMessage(const QString &errmsg, QString *file, QString *line, QString *severity, QString *text, QString *id)
{
    int pos1 = 0;
    int pos2 = 0;

    // filename
    while (pos2 < errmsg.size() && errmsg[pos2] != ':')
        ++pos2;
    if (pos2 >= errmsg.size())
        return false;
    *file = errmsg.mid(pos1,pos2-pos1);
    pos1 = ++pos2;

    // line
    while (pos2 < errmsg.size() && errmsg[pos2].isNumber())
        ++pos2;
    if (pos2 >= errmsg.size() || errmsg[pos2] != ':')
        return false;
    *line = errmsg.mid(pos1,pos2-pos1);
    pos1 = ++pos2;

    // column
    if (pos2 < errmsg.size() && errmsg[pos2].isNumber()) {
        while (pos2 < errmsg.size() && errmsg[pos2].isNumber())
            ++pos2;
        if (pos2 >= errmsg.size() || errmsg[pos2] != ':')
            return false;
        pos1 = ++pos2;
    }

    // severity
    while (pos2 < errmsg.size() && errmsg[pos2] == ' ')
        ++pos2;
    pos1 = pos2;
    while (pos2 < errmsg.size() && errmsg[pos2].isLetter())
        ++pos2;
    if (pos2 >= errmsg.size() || errmsg[pos2] != ':')
        return false;
    *severity = errmsg.mid(pos1,pos2-pos1);
    pos1 = ++pos2;

    // text
    while (pos2 < errmsg.size() && errmsg[pos2] == ' ')
        ++pos2;
    pos1 = pos2;
    *text = errmsg.mid(pos1);

    if (errmsg.endsWith("]")) {
        pos1 = text->lastIndexOf("[");
        *id = text->mid(pos1 + 1, text->size()-pos1-2);
        *text = text->mid(0, pos1-1);
    }

    return true;
}
