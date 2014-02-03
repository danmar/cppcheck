#include "projectlist.h"
#include <QDomDocument>
#include <QFile>
#include <QTextStream>

ProjectList::ProjectList()
{
}

ProjectList::ProjectList(const ProjectList &projectlist)
{
    foreach(const Project *project, projectlist.projects) {
        Project *myproject = new Project;
        *myproject = *project;
        projects.append(myproject);
    }
}

void ProjectList::swap(ProjectList &projectlist)
{
    QList<Project*> p = projects;
    projects = projectlist.projects;
    projectlist.projects = p;
}

void ProjectList::load(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QDomDocument doc;
    if (!doc.setContent(&file))
        return;

    const QDomElement rootElement = doc.documentElement();
    if (rootElement.tagName() != "solution")
        return;

    ProjectList projectlist;

    for (QDomElement e = rootElement.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        if (e.tagName() == "project") {
            Project *project = new Project();
            projectlist.projects.append(project);
            if (!project->load(e))
                return;
        } else {
            return;
        }
    }

    qDeleteAll(projects);
    projects = projectlist.projects;
    projectlist.projects.clear();
}

static void loaddata(const QDomElement &element, const char tag[], QStringList *stringList)
{
    const QDomElement e = element.firstChildElement(tag);
    if (e.isNull())
        return;
    for (QDomElement string = e.firstChildElement(); !string.isNull(); string = string.nextSiblingElement()) {
        if (string.tagName() == "string")
            *stringList << string.text();
    }
}

static bool getflag(const QDomElement &element, const char name1[], const char name2[])
{
    const QDomElement child1 = element.firstChildElement(name1);
    if (child1.isNull())
        return false;
    const QDomElement child2 = child1.firstChildElement(name2);
    if (child2.isNull())
        return false;
    return child2.text() == "true";
}

bool ProjectList::Project::load(const QDomElement &element)
{
    name = element.attribute("name");
    if (name.isEmpty())
        return false;
    path = element.attribute("path");
    loaddata(element, "defines", &defines);
    loaddata(element, "includes", &includes);
    clang.compiler   = getflag(element, "clang", "compiler");
    clang.analyser   = getflag(element, "clang", "analyser");
    cppcheck.enabled = getflag(element, "cppcheck", "enabled");
    gcc.enabled      = getflag(element, "gcc", "enabled");
    return true;
}

void ProjectList::save(const QString &filename) const
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QDomDocument doc;
    QDomNode node(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\""));
    doc.appendChild(node);
    QDomElement root = doc.createElement("solution");
    doc.appendChild(root);

    foreach(const Project *project, projects) {
        QDomElement projectElement = doc.createElement("project");
        projectElement.setAttribute("name", project->name);
        projectElement.setAttribute("path", project->path);
        root.appendChild(projectElement);
        if (!project->defines.isEmpty()) {
            QDomElement definesElement = doc.createElement("defines");
            projectElement.appendChild(definesElement);
            foreach(const QString def, project->defines) {
                QDomElement stringElement = doc.createElement("string");
                definesElement.appendChild(stringElement);
                stringElement.appendChild(doc.createTextNode(def));
            }
        }
        if (!project->includes.isEmpty()) {
            QDomElement includesElement = doc.createElement("includes");
            projectElement.appendChild(includesElement);
            foreach(const QString inc, project->includes) {
                QDomElement stringElement = doc.createElement("string");
                includesElement.appendChild(stringElement);
                stringElement.appendChild(doc.createTextNode(inc));
            }
        }


        // clang settings
        {
            QDomElement clangElement = doc.createElement("clang");
            projectElement.appendChild(clangElement);
            QDomElement flagElement;
            flagElement = doc.createElement("compiler");
            clangElement.appendChild(flagElement);
            flagElement.appendChild(doc.createTextNode(project->clang.compiler ? "true" : "false"));
            flagElement = doc.createElement("analyser");
            clangElement.appendChild(flagElement);
            flagElement.appendChild(doc.createTextNode(project->clang.analyser ? "true" : "false"));
        }

        // cppcheck settings
        {
            QDomElement cppcheckElement = doc.createElement("cppcheck");
            projectElement.appendChild(cppcheckElement);
            QDomElement flagElement = doc.createElement("enabled");
            cppcheckElement.appendChild(flagElement);
            flagElement.appendChild(doc.createTextNode(project->cppcheck.enabled ? "true" : "false"));
        }

        // gcc settings
        {
            QDomElement gccElement = doc.createElement("gcc");
            projectElement.appendChild(gccElement);
            QDomElement flagElement = doc.createElement("enabled");
            gccElement.appendChild(flagElement);
            flagElement.appendChild(doc.createTextNode(project->gcc.enabled ? "true" : "false"));
        }
    }

    QTextStream out(&file);
    doc.save(out, 2);
}

const ProjectList::Project *ProjectList::getproject(const QString &projectName) const
{
    foreach(const Project *project, projects) {
        if (project->name == projectName)
            return project;
    }
    return 0;
}

ProjectList::Project *ProjectList::getproject(const QString &projectName)
{
    foreach(Project *project, projects) {
        if (project->name == projectName)
            return project;
    }
    return 0;
}

QStringList ProjectList::projectNames() const
{
    QStringList ret;
    foreach(const Project *project, projects) {
        ret << project->name;
    }
    ret.sort();
    return ret;
}
