//
// Copyright (c) 2017-2020 the rbfx project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include <Urho3D/Core/Exception.h>
#include <Urho3D/Core/Object.h>

#include <EASTL/vector.h>

namespace Urho3D
{

/// Exception thrown when UndoManager stack is desynchronized with editor state.
class UndoException : public RuntimeException
{
public:
    using RuntimeException::RuntimeException;
};

/// ID corresponding to the temporal order of undo actions.
using EditorActionFrame = unsigned long long;

/// Abstract undoable and redoable action.
class EditorAction : public RefCounted
{
public:
    /// Return whether the action should be completely removed from stack on undo.
    /// Useful for injecting callback on undoing. Don't change any important state if true!
    virtual bool RemoveOnUndo() const { return false; }
    /// Check if action is valid and alive, i.e. Undo and Redo can be called.
    virtual bool IsAlive() const { return true; }
    /// Action is pushed to the stack.
    virtual void OnPushed(EditorActionFrame frame) {}
    /// Redo this action. May fail if external state has unexpectedly changed.
    virtual void Redo() const = 0;
    /// Undo this action. May fail if external state has unexpectedly changed.
    virtual void Undo() const = 0;
    /// Try to merge this action with another. Return true if successfully merged.
    virtual bool MergeWith(const EditorAction& other) { return false; }
};

/// Base class for action wrappers.
class BaseEditorActionWrapper : public EditorAction
{
public:
    explicit BaseEditorActionWrapper(SharedPtr<EditorAction> action);

    /// Implement EditorAction.
    /// @{
    bool RemoveOnUndo() const override;
    bool IsAlive() const override;
    void OnPushed(EditorActionFrame frame) override;
    void Redo() const override;
    void Undo() const override;
    bool MergeWith(const EditorAction& other) override;
    /// @}

protected:
    SharedPtr<EditorAction> action_;
};

using EditorActionPtr = SharedPtr<EditorAction>;

/// Manages undo stack and actions.
class UndoManager : public Object
{
    URHO3D_OBJECT(UndoManager, Object);

public:
    explicit UndoManager(Context* context);

    /// Force new frame. Call it on any resource save.
    void NewFrame();
    /// Push new action. May be merged with the top of the stack. Drops redo stack.
    EditorActionFrame PushAction(const EditorActionPtr& action);
    /// Try to undo action. May fail if external state changed.
    bool Undo();
    /// Try to redo action. May fail if external state changed.
    bool Redo();

private:
    struct ActionGroup
    {
        EditorActionFrame frame_{};
        ea::vector<EditorActionPtr> actions_;

        bool IsAlive() const;
    };

    bool NeedNewGroup() const;

    ea::vector<ActionGroup> undoStack_;
    ea::vector<ActionGroup> redoStack_;
    EditorActionFrame frame_{};
};

}