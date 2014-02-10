/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Build Suite.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/
#ifndef QBS_BUILDGRAPH_H
#define QBS_BUILDGRAPH_H

#include "forward_decls.h"
#include <language/forward_decls.h>

#include <QScriptValue>
#include <QStringList>

namespace qbs {
namespace Internal {
class BuildGraphNode;
class Logger;
class ScriptEngine;
class PrepareScriptObserver;

Artifact *lookupArtifact(const ResolvedProductConstPtr &product,
                         const ProjectBuildData *projectBuildData,
                         const QString &dirPath, const QString &fileName,
                         bool compareByName = false);
Artifact *lookupArtifact(const ResolvedProductConstPtr &product, const QString &dirPath,
                         const QString &fileName, bool compareByName = false);
Artifact *lookupArtifact(const ResolvedProductConstPtr &product, const ProjectBuildData *buildData,
                         const QString &filePath, bool compareByName = false);
Artifact *lookupArtifact(const ResolvedProductConstPtr &product, const QString &filePath,
                         bool compareByName = false);
Artifact *lookupArtifact(const ResolvedProductConstPtr &product, const Artifact *artifact,
                         bool compareByName);

Artifact *createArtifact(const ResolvedProductPtr &product,
                         const SourceArtifactConstPtr &sourceArtifact, const Logger &logger);
void insertArtifact(const ResolvedProductPtr &product, Artifact *artifact, const Logger &logger);
void dumpProductBuildData(const ResolvedProductConstPtr &product);


bool findPath(BuildGraphNode *u, BuildGraphNode *v, QList<BuildGraphNode*> &path);
void connect(BuildGraphNode *p, BuildGraphNode *c);
void loggedConnect(BuildGraphNode *u, BuildGraphNode *v, const Logger &logger);
bool safeConnect(Artifact *u, Artifact *v, const Logger &logger);
void removeGeneratedArtifactFromDisk(Artifact *artifact, const Logger &logger);
void removeGeneratedArtifactFromDisk(const QString &filePath, const Logger &logger);
void disconnect(BuildGraphNode *u, BuildGraphNode *v, const Logger &logger);

void setupScriptEngineForFile(ScriptEngine *engine, const ResolvedFileContextConstPtr &fileContext,
        QScriptValue targetObject);
void setupScriptEngineForProduct(ScriptEngine *engine, const ResolvedProductConstPtr &product,
                                 const RuleConstPtr &rule, QScriptValue targetObject,
                                 PrepareScriptObserver *observer = 0);
QString relativeArtifactFileName(const Artifact *artifact); // Debugging helpers

void doSanityChecks(const ResolvedProjectPtr &project, const Logger &logger);

} // namespace Internal
} // namespace qbs

#endif // QBS_BUILDGRAPH_H
