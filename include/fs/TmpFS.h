// Copyright (c) 2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VALKYRIE_TMP_FS_H_
#define VALKYRIE_TMP_FS_H_

#include <List.h>
#include <Memory.h>
#include <String.h>
#include <fs/File.h>
#include <fs/FileSystem.h>
#include <fs/Vnode.h>

namespace valkyrie::kernel {

// Forward declaration
class TmpFS;


class TmpFSVnode final : public Vnode {
  // Friend declaration
  friend class TmpFS;

 public:
  TmpFSVnode(TmpFS& fs,
             TmpFSVnode* parent,
             const String& name,
             const char* content,
             size_t len);

  virtual ~TmpFSVnode();


  virtual void add_child(SharedPtr<Vnode> child) override;
  virtual SharedPtr<Vnode> remove_child(const String& name) override;

  virtual int chmod(const mode_t mode) override;
  virtual int chown(const uid_t uid, const gid_t gid) override;

  virtual char* get_content() const override { return _content.get(); }
  const String& get_name() const { return _name; }

 private:
  String _name;
  UniquePtr<char[]> _content;

  TmpFSVnode* _parent;  // FIXME: use WeakPtr
  List<SharedPtr<TmpFSVnode>> _children;
};


class TmpFS final : public FileSystem {
  // Friend declaration
  friend class TmpFSVnode;
  friend class CPIOArchive;  // FIXME: wtf

 public:
  TmpFS();
  virtual ~TmpFS() = default;

  virtual SharedPtr<Vnode> create(const String& pathname,
                                  const char* content,
                                  size_t size,
                                  mode_t mode,
                                  uid_t uid,
                                  gid_t gid) override;

  virtual SharedPtr<File> open(const String& pathname, int flags) override;
  virtual int close(SharedPtr<File> file) override;
  virtual int write(SharedPtr<File> file, const void* buf, size_t len) override;
  virtual int read(SharedPtr<File> file, void* buf, size_t len) override;

  virtual void show() const override;
  virtual SharedPtr<Vnode> get_root_vnode() override;
  virtual SharedPtr<Vnode> get_vnode(const String& pathname) override;

 private:
  // FIXME: `inode` should be marked const, but it seems that
  // List<T>::ConstIterator isn't implemented properly...
  // I don't have the luxury (i.e. time) to fix this now, so maybe later...
  void debug_show_dfs_helper(TmpFSVnode* vnode, const int depth) const;

  uint64_t _next_vnode_index;
  SharedPtr<TmpFSVnode> _root_vnode;
};

}  // namespace valkyrie::kernel

#endif  // VALKYRIE_TMP_FS_H_
