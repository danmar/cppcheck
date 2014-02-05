#ifndef PROJECTLIST_H
#define PROJECTLIST_H

#include <QDomElement>
#include <QList>
#include <QString>
#include <QStringList>

class ProjectList {
public:
    ProjectList();
    ProjectList(const ProjectList &projectlist);
    ~ProjectList() {
        qDeleteAll(projects);
    }

    // Disable operator=() this is not implemented
    ProjectList& operator=(const ProjectList &);

    void swap(ProjectList &projectlist2);

    void load(const QString &filename);
    void save(const QString &filename) const;

    class Project {
    public:
        Project() {
            clang.compiler = true;
            clang.analyser = true;
            cppcheck.enabled = true;
            gcc.enabled = true;
        }
        bool load(const QDomElement &element);
        QString name;
        QString path;
        QStringList defines;
        QStringList includes;

        struct {
            bool compiler;
            bool analyser;
        } clang;
        struct {
            bool enabled;
        } cppcheck;
        struct {
            bool enabled;
        } gcc;
    };

    QStringList projectNames() const;
    const Project *getproject(const QString &projectName) const;
    Project *getproject(const QString &projectName);

    QList<Project*> projects;
};

#endif // PROJECTLIST_H
