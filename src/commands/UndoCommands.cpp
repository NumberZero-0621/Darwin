#include "UndoCommands.h"
#include "Project.h"
#include "Track.h"
#include "Clip.h"
#include "Note.h"
#include "VST3PluginInstance.h"

#include <QDebug>

// ===== ノート追加コマンド =====

AddNoteCommand::AddNoteCommand(Clip* clip, int pitch, qint64 startTick,
                               qint64 durationTicks, int velocity,
                               QUndoCommand* parent)
    : QUndoCommand("Add Note", parent)
    , m_clip(clip)
    , m_note(nullptr)
    , m_pitch(pitch)
    , m_startTick(startTick)
    , m_durationTicks(durationTicks)
    , m_velocity(velocity)
    , m_ownsNote(false)
    , m_firstRedo(true)
{
}

AddNoteCommand::~AddNoteCommand()
{
    if (m_ownsNote && m_note) {
        delete m_note;
    }
}

void AddNoteCommand::undo()
{
    if (m_note && m_clip) {
        m_clip->takeNote(m_note);
        m_ownsNote = true;
    }
}

void AddNoteCommand::redo()
{
    if (m_firstRedo) {
        if (m_clip) {
            m_note = m_clip->addNote(m_pitch, m_startTick, m_durationTicks, m_velocity);
            m_ownsNote = false;
        }
        m_firstRedo = false;
    } else {
        if (m_clip && m_note) {
            m_clip->insertNote(m_note);
            m_ownsNote = false;
        }
    }
}

// ===== ノート削除コマンド =====

RemoveNoteCommand::RemoveNoteCommand(Clip* clip, Note* note,
                                     QUndoCommand* parent)
    : QUndoCommand("Remove Note", parent)
    , m_clip(clip)
    , m_note(note)
    , m_ownsNote(false)
{
}

RemoveNoteCommand::~RemoveNoteCommand()
{
    if (m_ownsNote && m_note) {
        delete m_note;
    }
}

void RemoveNoteCommand::undo()
{
    if (m_clip && m_note) {
        m_clip->insertNote(m_note);
        m_ownsNote = false;
    }
}

void RemoveNoteCommand::redo()
{
    if (m_clip && m_note) {
        m_clip->takeNote(m_note);
        m_ownsNote = true;
    }
}

// ===== ノート移動コマンド =====
// (既存のまま)
MoveNoteCommand::MoveNoteCommand(Note* note, int newPitch,
                                 qint64 newStartTick, QUndoCommand* parent)
    : QUndoCommand("Move Note", parent)
    , m_note(note)
    , m_oldPitch(note->pitch())
    , m_oldStartTick(note->startTick())
    , m_newPitch(newPitch)
    , m_newStartTick(newStartTick)
{
}

void MoveNoteCommand::undo()
{
    m_note->setPitch(m_oldPitch);
    m_note->setStartTick(m_oldStartTick);
}

void MoveNoteCommand::redo()
{
    m_note->setPitch(m_newPitch);
    m_note->setStartTick(m_newStartTick);
}

bool MoveNoteCommand::mergeWith(const QUndoCommand* other)
{
    if (other->id() != id()) return false;
    auto* cmd = static_cast<const MoveNoteCommand*>(other);
    if (cmd->m_note != m_note) return false;
    m_newPitch = cmd->m_newPitch;
    m_newStartTick = cmd->m_newStartTick;
    return true;
}

// ===== ノートリサイズコマンド =====

ResizeNoteCommand::ResizeNoteCommand(Note* note, qint64 newDurationTicks,
                                     QUndoCommand* parent)
    : QUndoCommand("Resize Note", parent)
    , m_note(note)
    , m_oldDurationTicks(note->durationTicks())
    , m_newDurationTicks(newDurationTicks)
{
}

void ResizeNoteCommand::undo()
{
    m_note->setDurationTicks(m_oldDurationTicks);
}

void ResizeNoteCommand::redo()
{
    m_note->setDurationTicks(m_newDurationTicks);
}

bool ResizeNoteCommand::mergeWith(const QUndoCommand* other)
{
    if (other->id() != id()) return false;
    auto* cmd = static_cast<const ResizeNoteCommand*>(other);
    if (cmd->m_note != m_note) return false;
    m_newDurationTicks = cmd->m_newDurationTicks;
    return true;
}

// ===== ベロシティ変更コマンド =====

ChangeVelocityCommand::ChangeVelocityCommand(Note* note, int newVelocity,
                                             QUndoCommand* parent)
    : QUndoCommand("Change Velocity", parent)
    , m_note(note)
    , m_oldVelocity(note->velocity())
    , m_newVelocity(newVelocity)
{
}

void ChangeVelocityCommand::undo()
{
    m_note->setVelocity(m_oldVelocity);
}

void ChangeVelocityCommand::redo()
{
    m_note->setVelocity(m_newVelocity);
}

bool ChangeVelocityCommand::mergeWith(const QUndoCommand* other)
{
    if (other->id() != id()) return false;
    auto* cmd = static_cast<const ChangeVelocityCommand*>(other);
    if (cmd->m_note != m_note) return false;
    m_newVelocity = cmd->m_newVelocity;
    return true;
}

// ===== クリップ追加コマンド =====

AddClipCommand::AddClipCommand(Track* track, qint64 startTick,
                               qint64 durationTicks, QUndoCommand* parent)
    : QUndoCommand("Add Clip", parent)
    , m_track(track)
    , m_clip(nullptr)
    , m_startTick(startTick)
    , m_durationTicks(durationTicks)
    , m_ownsClip(false)
    , m_firstRedo(true)
{
}

AddClipCommand::~AddClipCommand()
{
    if (m_ownsClip && m_clip) {
        delete m_clip;
    }
}

void AddClipCommand::undo()
{
    if (m_clip && m_track) {
        m_track->takeClip(m_clip);
        m_ownsClip = true;
    }
}

void AddClipCommand::redo()
{
    if (m_firstRedo) {
        if (m_track) {
            m_clip = m_track->addClip(m_startTick, m_durationTicks);
            m_ownsClip = false;
        }
        m_firstRedo = false;
    } else {
        if (m_track && m_clip) {
            m_track->insertClip(m_clip);
            m_ownsClip = false;
        }
    }
}

// ===== クリップ削除コマンド =====

RemoveClipCommand::RemoveClipCommand(Track* track, Clip* clip,
                                     QUndoCommand* parent)
    : QUndoCommand("Remove Clip", parent)
    , m_track(track)
    , m_clip(clip)
    , m_ownsClip(false)
{
}

RemoveClipCommand::~RemoveClipCommand()
{
    if (m_ownsClip && m_clip) {
        delete m_clip;
    }
}

void RemoveClipCommand::undo()
{
    if (m_track && m_clip) {
        m_track->insertClip(m_clip);
        m_ownsClip = false;
    }
}

void RemoveClipCommand::redo()
{
    if (m_track && m_clip) {
        m_track->takeClip(m_clip);
        m_ownsClip = true;
    }
}

// ===== クリップ移動コマンド =====

MoveClipCommand::MoveClipCommand(Clip* clip, qint64 newStartTick,
                                 QUndoCommand* parent)
    : QUndoCommand("Move Clip", parent)
    , m_clip(clip)
    , m_oldStartTick(clip->startTick())
    , m_newStartTick(newStartTick)
{
}

void MoveClipCommand::undo()
{
    m_clip->setStartTick(m_oldStartTick);
}

void MoveClipCommand::redo()
{
    m_clip->setStartTick(m_newStartTick);
}

bool MoveClipCommand::mergeWith(const QUndoCommand* other)
{
    if (other->id() != id()) return false;
    auto* cmd = static_cast<const MoveClipCommand*>(other);
    if (cmd->m_clip != m_clip) return false;
    m_newStartTick = cmd->m_newStartTick;
    return true;
}

// ===== クリップリサイズコマンド =====

ResizeClipCommand::ResizeClipCommand(Clip* clip, qint64 newDurationTicks,
                                     QUndoCommand* parent)
    : QUndoCommand("Resize Clip", parent)
    , m_clip(clip)
    , m_oldDurationTicks(clip->durationTicks())
    , m_newDurationTicks(newDurationTicks)
{
}

void ResizeClipCommand::undo()
{
    m_clip->setDurationTicks(m_oldDurationTicks);
}

void ResizeClipCommand::redo()
{
    m_clip->setDurationTicks(m_newDurationTicks);
}

bool ResizeClipCommand::mergeWith(const QUndoCommand* other)
{
    if (other->id() != id()) return false;
    auto* cmd = static_cast<const ResizeClipCommand*>(other);
    if (cmd->m_clip != m_clip) return false;
    m_newDurationTicks = cmd->m_newDurationTicks;
    return true;
}

// ===== トラック追加コマンド =====

AddTrackCommand::AddTrackCommand(Project* project, const QString& name,
                                 QUndoCommand* parent)
    : QUndoCommand("Add Track", parent)
    , m_project(project)
    , m_track(nullptr)
    , m_name(name)
    , m_ownsTrack(false)
    , m_firstRedo(true)
{
}

AddTrackCommand::~AddTrackCommand()
{
    if (m_ownsTrack && m_track) {
        delete m_track;
    }
}

void AddTrackCommand::undo()
{
    if (m_track && m_project) {
        m_project->takeTrack(m_track);
        m_ownsTrack = true;
    }
}

void AddTrackCommand::redo()
{
    if (m_firstRedo) {
        if (m_project) {
            m_track = m_project->addTrack(m_name);
            m_ownsTrack = false;
        }
        m_firstRedo = false;
    } else {
        if (m_project && m_track) {
            m_project->insertTrack(m_track);
            m_ownsTrack = false;
        }
    }
}

// ===== トラック削除コマンド =====

RemoveTrackCommand::RemoveTrackCommand(Project* project, Track* track,
                                       QUndoCommand* parent)
    : QUndoCommand("Remove Track", parent)
    , m_project(project)
    , m_track(track)
    , m_trackIndex(project->tracks().indexOf(track))
    , m_ownsTrack(false)
{
}

RemoveTrackCommand::~RemoveTrackCommand()
{
    if (m_ownsTrack && m_track) {
        delete m_track;
    }
}

void RemoveTrackCommand::undo()
{
    if (m_project && m_track) {
        m_project->insertTrack(m_track, m_trackIndex);
        m_ownsTrack = false;
    }
}

void RemoveTrackCommand::redo()
{
    if (m_project && m_track) {
        m_project->takeTrack(m_track);
        m_ownsTrack = true;
    }
}
