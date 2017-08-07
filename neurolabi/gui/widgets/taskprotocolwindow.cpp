#include <iostream>
#include <stdlib.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QsLog.h>

#include "neutube.h"
#include "flyem/zflyemproofdoc.h"
#include "protocols/taskbodyreview.h"
#include "protocols/taskgototarget.h"

#include "taskprotocolwindow.h"
#include "ui_taskprotocolwindow.h"

TaskProtocolWindow::TaskProtocolWindow(ZFlyEmProofDoc *doc, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskProtocolWindow)
{
    ui->setupUi(this);

    m_proofDoc = doc;

    m_protocolInstanceStatus = UNCHECKED;

    // UI connections
    connect(ui->doneButton, SIGNAL(clicked(bool)), this, SLOT(onDoneButton()));
    connect(ui->loadTasksButton, SIGNAL(clicked(bool)), this, SLOT(onLoadTasksButton()));
    connect(ui->gotoButton, SIGNAL(clicked(bool)), this, SLOT(onGotoButton()));
    connect(ui->completedCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onCompletedStateChanged(int)));

    // start to do stuff
    if (!m_writer.open(doc->getDvidTarget())) {
        showError("Couldn't open DVID", "DVID couldn't be opened!  Check your network connections.");
        setWindowConfiguration(LOAD_BUTTON);
        return;
    }

    // check DVID; if user has a started task list, load it immediately
    QJsonObject json = loadJsonFromDVID(PROTOCOL_INSTANCE, generateDataKey());
    if (!json.isEmpty()) {
        // don't need to save to DVID if we load from DVID
        startProtocol(json, false);
    } else {
        // otherwise, show the load task file button
        setWindowConfiguration(LOAD_BUTTON);
    }
}
// constants
const QString TaskProtocolWindow::KEY_DESCRIPTION = "file type";
const QString TaskProtocolWindow::VALUE_DESCRIPTION = "Neu3 task list";
const QString TaskProtocolWindow::KEY_VERSION = "file version";
const int TaskProtocolWindow::currentVersion = 1;
const QString TaskProtocolWindow::KEY_TASKLIST = "task list";
const QString TaskProtocolWindow::KEY_TASKTYPE = "task type";
const QString TaskProtocolWindow::PROTOCOL_INSTANCE = "Neu3-protocols";
const QString TaskProtocolWindow::TASK_PROTOCOL_KEY = "task-protocol";

void TaskProtocolWindow::onDoneButton() {
    std::cout << "onDoneButton()" << std::endl;

    setWindowConfiguration(LOAD_BUTTON);

}

void TaskProtocolWindow::onLoadTasksButton() {
    // prompt for file path; might need to adjust this after testing on
    //  Linux; not sure what default file type filter is?
    QString result = QFileDialog::getOpenFileName(this, "Open task json file");
    if (result.size() == 0) {
        // canceled
        return;
    }

    // load json from file (for now; eventually, allow user to browse from DVID,
    //  or maybe enter an assignment ID or something)
    QJsonObject json = loadJsonFromFile(result);
    startProtocol(json, true);
}

void TaskProtocolWindow::onGotoButton() {
    gotoCurrentTask();
}

int TaskProtocolWindow::onCompletedStateChanged(int state) {
    if (m_currentTaskIndex >= 0) {
        m_taskList[m_currentTaskIndex]->setCompleted(ui->completedCheckBox->isChecked());
        saveState();
    }
}

/*
 * input: json from file or dvid; flag whether to save immediately back to dvid
 * output: none
 * effect: start up the UI for the protocol; finds first uncompleted task and loads
 *      into UI
 */
void TaskProtocolWindow::startProtocol(QJsonObject json, bool save) {

    // validate json; this call displays errors itself
    if (!isValidJson(json)) {
        return;
    }

    // at the point in time we have older versions hanging around, this is where you
    //  would convert them


    // load tasks from json into internal data structures; save to DVID if needed
    loadTasks(json);
    if (save) {
        saveState();
    }

    // load first task; enable UI and go

    // if I introduce the "show completed" checkbox, this should
    //  change to "get first visible"

    m_currentTaskIndex = getFirstUncompleted();
    if (m_currentTaskIndex < 0) {
        showInfo("No tasks to do!", "All tasks have been completed!");
    }

    updateCurrentTaskLabel();
    updateLabel();
    setWindowConfiguration(TASK_UI);
}

void TaskProtocolWindow::gotoCurrentTask() {
    if (m_currentTaskIndex >= 0) {
        TaskGotoTarget target = m_taskList[m_currentTaskIndex]->gotoTarget();
        if (target.targetType() == TaskGotoTarget::BODY) {

            // go to body ID
            std::cout << "pretending to go to body " << target.bodyID() << std::endl;

        } else if (target.targetType() == TaskGotoTarget::POINT) {

            // go to point
            std::cout << "pretending to go to point " << target.point().toString() << std::endl;

        }
    }
}

/*
 * returns index of first uncompleted task, or -1
 */
int TaskProtocolWindow::getFirstUncompleted() {
    for (int i=0; i<m_taskList.size(); i++) {
        if (!m_taskList[i]->completed()) {
            return i;
        }
    }
    return -1;
}

/*
 * updates the task label for current index
 */
void TaskProtocolWindow::updateCurrentTaskLabel() {
    if (m_currentTaskIndex < 0) {
        ui->taskActionLabel->setText("(no task)");
        ui->taskTargetLabel->setText("n/a");
        ui->completedCheckBox->setChecked(false);
    } else {
        ui->taskActionLabel->setText(m_taskList[m_currentTaskIndex]->actionString());
        ui->taskTargetLabel->setText(m_taskList[m_currentTaskIndex]->targetString());
        ui->completedCheckBox->setChecked(m_taskList[m_currentTaskIndex]->completed());
    }
}

/*
 * updates any progress labels
 */
void TaskProtocolWindow::updateLabel() {
    int ncomplete = 0;
    foreach (QSharedPointer<TaskProtocolTask> task, m_taskList) {
        if (task->completed()) {
            ncomplete++;
        }
    }
    int ntasks = m_taskList.size();
    float percent = (100.0 * ncomplete) / ntasks;
    ui->progressLabel->setText(QString("%1 / %2 (%3%)").arg(ncomplete).arg(ntasks).arg(percent));
}

/*
 * save the internal data to dvid in predetermined instance and key
 * of current dvid target
 */
void TaskProtocolWindow::saveState() {
    QJsonObject tasks = storeTasks();
    saveJsonToDvid(tasks);
}

/*
 * input: filepath
 * output: json object of json in file; empty json if error
 * effect: shows error dialogs on errors
 */
QJsonObject TaskProtocolWindow::loadJsonFromFile(QString filepath) {
    QJsonObject emptyResult;

    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        showError("Error loading file", "Couldn't open file " + filepath + "!");
        return emptyResult;
        }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() or !doc.isObject()) {
        showError("Error parsing file", "Couldn't parse file " + filepath + "!");
        return emptyResult;
    } else {
        LINFO() << "Task protocol: json loaded from file " + filepath;
        return doc.object();
    }
}

/*
 * input: dvid instance and key names
 * output: json object of json in from dvid; empty json if error
 * effect: shows error dialogs on errors
 */
QJsonObject TaskProtocolWindow::loadJsonFromDVID(QString instance, QString key) {
    QJsonObject emptyResult;
    ZDvidReader reader;
    if (!reader.open(m_proofDoc->getDvidTarget())) {
        return emptyResult;
    }
    if (!reader.hasKey(instance, key)) {
        return emptyResult;
    }

    // we got something!  reel it in...
    QByteArray data = reader.readKeyValue(instance, key);
    QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() or !doc.isObject()) {
            showError("Error parsing JSON", "Couldn't parse JSON from " + instance +
                ", " + key + "!");
            return emptyResult;
        } else {
            LINFO() << "Task protocol: json loaded from " + instance + ", " + key;
            return doc.object();
        }
}

/*
 * check input json for validity; not comprehensive, but good enough for our purposes
 */
bool TaskProtocolWindow::isValidJson(QJsonObject json) {

    // check file description and version
    if (!json.contains(KEY_DESCRIPTION) || json[KEY_DESCRIPTION].toString() != VALUE_DESCRIPTION) {
        showError("Json parsing error", "This file does not appear to be a Neu3 task list file!");
        return false;
    }

    if (!json.contains(KEY_VERSION)) {
        showError("Json parsing error", "No version info in file!");
        return false;
    }

    int fileVersion = json[VALUE_DESCRIPTION].toInt();
    if (fileVersion > currentVersion) {
        showError("Json parsing issue", "This file is from a newer version of this software!  Update and try again.");
        return false;
    }

    if (!json.contains(KEY_TASKLIST)) {
        showError("Json parsing issue", "Can't find list of tasks in json file!");
        return false;
    }
    // could validate that it's a list and each element is a map, but I'll
    //  draw the line here for now

    return true;
}

/*
 * input: json object
 * effect: load data from json into internal data structures
 */
void TaskProtocolWindow::loadTasks(QJsonObject json) {

    m_taskList.clear();
    foreach(QJsonValue taskJson, json[KEY_TASKLIST].toArray()) {
        if (!taskJson.isObject()) {
            LWARN() << "Task protocol: found task json that is not an object; skipping";
            continue;
        }

        // this if-else tree will get more awkward with more types...

        // also, need to collect these keys and values better; hard-code this one
        //  for now
        QString taskType = taskJson.toObject()[KEY_TASKTYPE].toString();
        if (taskType == "body review") {
            QSharedPointer<TaskProtocolTask> task(new TaskBodyReview(taskJson.toObject()));
            m_taskList.append(task);
        } else {
            // unknown task type; log it and move on
            LWARN() << "Task protocol: found unknown task type " << taskType << " in task json; skipping";
        }
    }

    LINFO() << "Task protocol: loaded " << m_taskList.size() << " tasks";
}

/*
 * output: json object containing data from internal data structures
 */
QJsonObject TaskProtocolWindow::storeTasks() {

    QJsonObject json;
    json[KEY_DESCRIPTION] = VALUE_DESCRIPTION;
    json[KEY_VERSION] = currentVersion;

    QJsonArray tasks;
    foreach (QSharedPointer<TaskProtocolTask> task, m_taskList) {
        tasks.append(task->toJson());
    }
    json[KEY_TASKLIST] = tasks;

    return json;
}

/*
 * input: json object
 * effect: save json to dvid in predetermined instance and key
 */
void TaskProtocolWindow::saveJsonToDvid(QJsonObject json) {
    // check that instance exists; if not, create it
    if (!checkCreateDataInstance()) {
        showError("DVID error", "Could not create the protocol instance in DVID!  Data is not saved!");
        return;
    }

    QJsonDocument doc(json);
    QString jsonString(doc.toJson(QJsonDocument::Compact));
    m_writer.writeJsonString(PROTOCOL_INSTANCE.toStdString(), generateDataKey().toStdString(),
        jsonString.toStdString());

    LINFO() << "Task protocol: saved data to DVID";
}

/*
 * output: key under which protocol data should be stored in dvid
 */
QString TaskProtocolWindow::generateDataKey() {
    return QString::fromStdString(NeuTube::GetCurrentUserName()) + "-" + TASK_PROTOCOL_KEY;
}

/*
 * output: success or not
 * effect: check to see if predetermined instance exists in
 * dvid in current target; if not, attempt to create it (once)
 */
bool TaskProtocolWindow::checkCreateDataInstance() {
    if (m_protocolInstanceStatus == CHECKED_PRESENT) {
        return true;
    } else if (m_protocolInstanceStatus == CHECKED_ABSENT) {
        // only check once; always an error if we can't create
        //  the first time
        return false;
    }

    // m_protocolInstanceStatus = UNCHECKED:
    ZDvidReader reader;
    reader.setVerbose(false);
    if (reader.open(m_proofDoc->getDvidTarget())) {
        if (!reader.hasData(PROTOCOL_INSTANCE.toStdString())) {
            m_writer.createKeyvalue(PROTOCOL_INSTANCE.toStdString());
            // did it actually create?  I'm only going to try once
            if (reader.hasData(PROTOCOL_INSTANCE.toStdString())) {
                m_protocolInstanceStatus = CHECKED_PRESENT;
                return true;
            } else {
                m_protocolInstanceStatus = CHECKED_ABSENT;
                return false;
            }
        } else {
            m_protocolInstanceStatus = CHECKED_PRESENT;
            return true;
        }
    } else {
        m_protocolInstanceStatus = CHECKED_ABSENT;
        return false;
    }
}

/*
 * hide and show UI panels to correspond to "ready for loading data"
 * and "data loaded, time to do work"
 */
void TaskProtocolWindow::setWindowConfiguration(WindowConfigurations config) {
    // note that size constraint = fixed size, set in designer,
    //  ensures the widget resizes when child widgets are hidden/shown

    if (config == TASK_UI) {
        ui->loadTasksWidget->hide();
        ui->taskButtonsWidget->show();
        ui->taskDetailsWidget->show();
        ui->tasksProgressWidget->show();
    } else if (config == LOAD_BUTTON) {
        ui->loadTasksWidget->show();
        ui->taskButtonsWidget->hide();
        ui->taskDetailsWidget->hide();
        ui->tasksProgressWidget->hide();
    }
}

/*
 * input: title and message for error dialog
 * effect: shows error dialog (convenience function)
 */
void TaskProtocolWindow::showError(QString title, QString message) {
    QMessageBox errorBox;
    errorBox.setText(title);
    errorBox.setInformativeText(message);
    errorBox.setStandardButtons(QMessageBox::Ok);
    errorBox.setIcon(QMessageBox::Warning);
    errorBox.exec();
}

/*
 * input: title and message for info dialog
 * effect: shows info dialog (convenience function)
 */
void TaskProtocolWindow::showInfo(QString title, QString message) {
    QMessageBox infoBox;
    infoBox.setText(title);
    infoBox.setInformativeText(message);
    infoBox.setStandardButtons(QMessageBox::Ok);
    infoBox.setIcon(QMessageBox::Information);
    infoBox.exec();
}

TaskProtocolWindow::~TaskProtocolWindow()
{
    delete ui;
}
