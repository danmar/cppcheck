#include "resultsmodel.h"
#include <QBrush>
#include <QDate>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QTextStream>

ResultsModel::ResultsModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    rootNode = 0;
    modified = false;
}

void ResultsModel::clear()
{
    beginResetModel();
    delete rootNode;
    rootNode = 0;
    endResetModel();
    modified = false;
    currentFileName.clear();
    currentProjectName.clear();
}

void ResultsModel::addresult(const QString &path, const QString &errmsg)
{
    if (errmsg.isEmpty())
        return;

    QString file, line, severity, text, id;
    if (parseErrorMessage(errmsg, &file, &line, &severity, &text, &id)) {
        beginResetModel();
        if (!rootNode)
            rootNode = new Node;
        Node *node = new Node(rootNode,file,line,severity,text,id,"");
        if (file.replace('\\','/').startsWith(path))
            node->filename = node->filename.mid(path.size());
        endResetModel();
    }
}

void ResultsModel::hideId(int row)
{
    if (!rootNode)
        return;
    beginResetModel();
    Node *node = rootNode->shownchildren[row];
    const QString id = node->id;
    for (int i = rootNode->shownchildren.size() - 1; i >= 0; i--) {
        if (rootNode->shownchildren[i]->id == id)
            rootNode->shownchildren.removeAt(i);
    }
    endResetModel();
}

void ResultsModel::hideAllOtherId(int row)
{
    if (!rootNode)
        return;
    beginResetModel();
    Node *node = rootNode->shownchildren[row];
    const QString id = node->id;
    for (int i = rootNode->shownchildren.size() - 1; i >= 0; i--) {
        if (rootNode->shownchildren[i]->id != id)
            rootNode->shownchildren.removeAt(i);
    }
    endResetModel();
}

void ResultsModel::showAll()
{
    if (!rootNode)
        return;
    beginResetModel();
    rootNode->shownchildren = rootNode->allchildren;
    endResetModel();
}

void ResultsModel::diffAgainstFile(const QString &filename)
{
    if (!rootNode)
        return;

    QList<QString> results;
    for (int i = 0; i < rootNode->allchildren.size(); ++i) {
        results.append(rootNode->allchildren[i]->id + "," +
                       rootNode->allchildren[i]->line + "," +
                       rootNode->allchildren[i]->filename);
    }

    ResultsModel other;
    other.load(filename,QString());
    if (!other.rootNode)
        return;

    QList<QString> otherResults;
    for (int i = 0; i < other.rootNode->allchildren.size(); ++i) {
        otherResults.append(other.rootNode->allchildren[i]->id + "," +
                            other.rootNode->allchildren[i]->line + "," +
                            other.rootNode->allchildren[i]->filename);
    }

    beginResetModel();
    rootNode->shownchildren.clear();
    for (int i = 0; i < results.size(); ++i) {
        if (!otherResults.contains(results[i])) {
            rootNode->shownchildren.append(rootNode->allchildren[i]);
        }
        if (!results.contains(otherResults[i])) {
            rootNode->shownchildren.append(other.rootNode->allchildren[i]);
            other.rootNode->allchildren[i] = 0;
        }
    }
    endResetModel();
}


static QString getstr(const QDomElement element, const QString &tagName)
{
    const QDomElement child = element.firstChildElement(tagName);
    return child.isNull() ? QString() : child.text();
}

bool ResultsModel::load(const QString &fileName, const QString &projectPath)
{
    modified = false;
    currentFileName = fileName;
    currentProjectName.clear();

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QDomDocument doc;
    if (!doc.setContent(&file))
        return false;

    const QDomElement rootElement = doc.documentElement();
    if (rootElement.tagName() != "results")
        return false;

    const QDomElement metaElement = rootElement.firstChildElement("meta");
    if (!metaElement.isNull())
        currentProjectName = getstr(metaElement, "project");

    const QDomElement resultsElement = rootElement.firstChildElement("results");
    if (resultsElement.isNull())
        return false;

    beginResetModel();

    delete rootNode;
    rootNode = 0;

    for (QDomElement element = resultsElement.firstChildElement(); !element.isNull(); element = element.nextSiblingElement()) {
        if (element.tagName() == "result") {
            if (!rootNode)
                rootNode = new Node;

            Node *node = new Node(rootNode,
                                  getstr(element,"file"),
                                  getstr(element,"line"),
                                  getstr(element,"severity"),
                                  getstr(element,"text"),
                                  getstr(element,"id"),
                                  getstr(element,"triage"));
            if (!node->filename.isEmpty())
                node->fullfilename = projectPath + "/" + node->filename;
        }
    }

    endResetModel();
    return true;
}

bool ResultsModel::save(const QString &fileName, const QString &projectName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QDomDocument doc;
    QDomElement root = doc.createElement("results");
    doc.appendChild(root);

    QDomElement meta = doc.createElement("meta");
    QDomElement project = doc.createElement("project");
    QDomElement date    = doc.createElement("date");

    root.appendChild(meta);
    meta.appendChild(project);
    project.appendChild(doc.createTextNode(projectName));
    meta.appendChild(date);
    date.appendChild(doc.createTextNode(QDate::currentDate().toString("yyyy-MM-dd")));

    QDomElement results = doc.createElement("results");
    root.appendChild(results);
    foreach(const Node *node, rootNode ? rootNode->allchildren : QList<Node*>()) {
        QDomElement result = doc.createElement("result");
        QDomElement file     = doc.createElement("file");
        QDomElement line     = doc.createElement("line");
        QDomElement severity = doc.createElement("severity");
        QDomElement text     = doc.createElement("text");
        QDomElement id       = doc.createElement("id");
        QDomElement triage   = doc.createElement("triage");
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

        result.appendChild(triage);
        if (node->triage == Node::FALSE_POSITIVE)
            triage.appendChild(doc.createTextNode("false positive"));
        else if (node->triage == Node::TRUE_POSITIVE)
            triage.appendChild(doc.createTextNode("true positive"));
        else
            triage.appendChild(doc.createTextNode("unknown"));
    }

    QTextStream out(&file);
    doc.save(out, 4);

    modified = false;
    currentFileName = fileName;
    currentProjectName = projectName;

    return true;
}

void ResultsModel::saveIfModified()
{
    if (modified && !currentFileName.isEmpty() && !currentProjectName.isEmpty()) {
        modified = false;
        save(currentFileName,currentProjectName);
    }
}


QModelIndex ResultsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!rootNode)
        return QModelIndex();
    Node *parentNode = nodeFromIndex(parent);
    return createIndex(row, column, parentNode->shownchildren[row]);
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
    return parentNode->shownchildren.count();
}

int ResultsModel::columnCount(const QModelIndex & /* parent */) const
{
    return 6;
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
    int row = grandparentNode->shownchildren.indexOf(parentNode);
    return createIndex(row, child.column(), parentNode);
}

QVariant ResultsModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {

        const Node *node = nodeFromIndex(index);
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
        case 5:
            if (node->triage == ResultsModel::Node::TRUE_POSITIVE)
                return "True positive";
            else if (node->triage == ResultsModel::Node::FALSE_POSITIVE)
                return "False positive";
            else
                return QVariant();
        default:
            return QVariant();
        }
    } else if (role == Qt::BackgroundRole) {
        const Node *node = nodeFromIndex(index);
        if (!node)
            return QVariant();
        if (node->triage == ResultsModel::Node::TRUE_POSITIVE)
            return QBrush(Qt::red);
        if (node->triage == ResultsModel::Node::FALSE_POSITIVE)
            return QBrush(Qt::gray);
    }

    return QVariant();
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
        case 5:
            return tr("Triage");
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
    if (errmsg.size() > 3 && errmsg[0].isLetter() && errmsg[1]==':' && (errmsg[2]=='\\' || errmsg[2]=='/'))
        pos2 = 3;
    while (pos2 < errmsg.size() && errmsg[pos2] != ':')
        ++pos2;
    if (pos2 >= errmsg.size())
        return false;
    *file = errmsg.mid(pos1,pos2-pos1);
    file->replace('\\','/');
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
    if (pos2 < errmsg.size() && errmsg[pos2] == ' ') {
        ++pos2;
        while (pos2 < errmsg.size() && errmsg[pos2].isLetter())
            ++pos2;
    }
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
