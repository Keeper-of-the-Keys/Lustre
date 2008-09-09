/* -*- mode: c; c-basic-offset: 8; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see
 * http://www.sun.com/software/products/lustre/docs/GPLv2.pdf
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *
 * GPL HEADER END
 */
/*
 * Copyright  2008 Sun Microsystems, Inc. All rights reserved
 * Use is subject to license terms.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 *
 * lustre/include/md_object.h
 *
 * Extention of lu_object.h for metadata objects
 */

#ifndef _LUSTRE_MD_OBJECT_H
#define _LUSTRE_MD_OBJECT_H

/*
 * Sub-class of lu_object with methods common for "meta-data" objects in MDT
 * stack.
 *
 * Meta-data objects implement namespace operations: you can link, unlink
 * them, and treat them as directories.
 *
 * Examples: mdt, cmm, and mdt are implementations of md interface.
 */


/*
 * super-class definitions.
 */
#include <lu_object.h>
#include <lvfs.h>

struct md_device;
struct md_device_operations;
struct md_object;


enum {
        UCRED_INVALID   = -1,
        UCRED_INIT      = 0,
        UCRED_OLD       = 1,
        UCRED_NEW       = 2
};

struct md_ucred {
        __u32               mu_valid;
        __u32                   mu_o_uid;
        __u32                   mu_o_gid;
        __u32                   mu_o_fsuid;
        __u32                   mu_o_fsgid;
        __u32                   mu_uid;
        __u32                   mu_gid;
        __u32                   mu_fsuid;
        __u32                   mu_fsgid;
        __u32                   mu_suppgids[2];
        cfs_cap_t               mu_cap;
        __u32                   mu_umask;
	struct group_info      *mu_ginfo;
	struct md_identity *mu_identity;
};

#define MD_CAPAINFO_MAX 5

/* there are at most 5 fids in one operation, see rename, NOTE the last one
 * is a temporary one used for is_subdir() */
struct md_capainfo {
        const struct lu_fid    *mc_fid[MD_CAPAINFO_MAX];
        struct lustre_capa     *mc_capa[MD_CAPAINFO_MAX];
};

/*
 * Implemented in mdd/mdd_handler.c.
 *
 * XXX should be moved into separate .h/.c together with all md security
 * related definitions.
 */
struct md_ucred *md_ucred(const struct lu_env *env);
struct md_capainfo *md_capainfo(const struct lu_env *env);

/* metadata attributes */
enum ma_valid {
        MA_INODE     = (1 << 0),
        MA_LOV       = (1 << 1),
        MA_COOKIE    = (1 << 2),
        MA_FLAGS     = (1 << 3),
        MA_LMV       = (1 << 4),
        MA_ACL_DEF   = (1 << 5),
        MA_LOV_DEF   = (1 << 6)
};

typedef enum {
        MDL_MINMODE  = 0,
        MDL_EX       = 1,
        MDL_PW       = 2,
        MDL_PR       = 4,
        MDL_CW       = 8,
        MDL_CR       = 16,
        MDL_NL       = 32,
        MDL_GROUP    = 64,
        MDL_MAXMODE
} mdl_mode_t;

typedef enum {
        MDT_NUL_LOCK = 0,
        MDT_REG_LOCK = (1 << 0),
        MDT_PDO_LOCK = (1 << 1)
} mdl_type_t;

struct md_attr {
        __u64                   ma_valid;
        __u64                   ma_need;
        __u64                   ma_attr_flags;
        struct lu_attr          ma_attr;
        struct lov_mds_md      *ma_lmm;
        int                     ma_lmm_size;
        struct lmv_stripe_md   *ma_lmv;
        int                     ma_lmv_size;
        void                   *ma_acl;
        int                     ma_acl_size;
        struct llog_cookie     *ma_cookie;
        int                     ma_cookie_size;
        struct lustre_capa     *ma_capa;
};

/* Additional parameters for create */
struct md_op_spec {
        union {
                /* symlink target */
                const char               *sp_symname;
                /* parent FID for cross-ref mkdir */
                const struct lu_fid      *sp_pfid;
                /* eadata for regular files */
                struct md_spec_reg {
                        /* lov objs exist already */
                        const struct lu_fid   *fid;
                        int no_lov_create;
                        const void *eadata;
                        int  eadatalen;
                } sp_ea;
        } u;
        
        /* Create flag from client: such as MDS_OPEN_CREAT, and others. */
        __u32      sp_cr_flags;

        /* Should mdd do lookup sanity check or not. */
        int        sp_cr_lookup;

        /* Current lock mode for parent dir where create is performing. */
        mdl_mode_t sp_cr_mode;

        /* Check for split */
        int        sp_ck_split;
};

/*
 * Operations implemented for each md object (both directory and leaf).
 */
struct md_object_operations {
        int (*moo_permission)(const struct lu_env *env,
                              struct md_object *pobj, struct md_object *cobj,
                              struct md_attr *attr, int mask);

        int (*moo_attr_get)(const struct lu_env *env, struct md_object *obj,
                            struct md_attr *attr);

        int (*moo_attr_set)(const struct lu_env *env, struct md_object *obj,
                            const struct md_attr *attr);

        int (*moo_xattr_get)(const struct lu_env *env, struct md_object *obj,
                             struct lu_buf *buf, const char *name);

        int (*moo_xattr_list)(const struct lu_env *env, struct md_object *obj,
                              struct lu_buf *buf);

        int (*moo_xattr_set)(const struct lu_env *env, struct md_object *obj,
                             const struct lu_buf *buf, const char *name,
                             int fl);

        int (*moo_xattr_del)(const struct lu_env *env, struct md_object *obj,
                             const char *name);

        int (*moo_readpage)(const struct lu_env *env, struct md_object *obj,
                            const struct lu_rdpg *rdpg);

        int (*moo_readlink)(const struct lu_env *env, struct md_object *obj,
                            struct lu_buf *buf);

        /* part of cross-ref operation */
        int (*moo_object_create)(const struct lu_env *env,
                                 struct md_object *obj,
                                 const struct md_op_spec *spec,
                                 struct md_attr *ma);

        int (*moo_ref_add)(const struct lu_env *env,
                           struct md_object *obj,
                           const struct md_attr *ma);

        int (*moo_ref_del)(const struct lu_env *env,
                           struct md_object *obj,
                           struct md_attr *ma);

        int (*moo_open)(const struct lu_env *env,
                        struct md_object *obj, int flag);

        int (*moo_close)(const struct lu_env *env, struct md_object *obj,
                         struct md_attr *ma);
        
        int (*moo_capa_get)(const struct lu_env *, struct md_object *,
                            struct lustre_capa *, int renewal);
        int (*moo_object_sync)(const struct lu_env *, struct md_object *);
};

/*
 * Operations implemented for each directory object.
 */
struct md_dir_operations {
        int (*mdo_is_subdir) (const struct lu_env *env, struct md_object *obj,
                              const struct lu_fid *fid, struct lu_fid *sfid);

        int (*mdo_lookup)(const struct lu_env *env, struct md_object *obj,
                          const struct lu_name *lname, struct lu_fid *fid,
                          struct md_op_spec *spec);

        mdl_mode_t (*mdo_lock_mode)(const struct lu_env *env,
                                    struct md_object *obj,
                                    mdl_mode_t mode);

        int (*mdo_create)(const struct lu_env *env, struct md_object *pobj,
                          const struct lu_name *lname, struct md_object *child,
                          struct md_op_spec *spec,
                          struct md_attr *ma);

        /* This method is used for creating data object for this meta object*/
        int (*mdo_create_data)(const struct lu_env *env, struct md_object *p,
                               struct md_object *o,
                               const struct md_op_spec *spec,
                               struct md_attr *ma);

        int (*mdo_rename)(const struct lu_env *env, struct md_object *spobj,
                          struct md_object *tpobj, const struct lu_fid *lf,
                          const struct lu_name *lsname, struct md_object *tobj,
                          const struct lu_name *ltname, struct md_attr *ma);

        int (*mdo_link)(const struct lu_env *env, struct md_object *tgt_obj,
                        struct md_object *src_obj, const struct lu_name *lname,
                        struct md_attr *ma);

        int (*mdo_unlink)(const struct lu_env *env, struct md_object *pobj,
                          struct md_object *cobj, const struct lu_name *lname,
                          struct md_attr *ma);

        /* partial ops for cross-ref case */
        int (*mdo_name_insert)(const struct lu_env *env,
                               struct md_object *obj,
                               const struct lu_name *lname,
                               const struct lu_fid *fid,
                               const struct md_attr *ma);

        int (*mdo_name_remove)(const struct lu_env *env,
                               struct md_object *obj,
                               const struct lu_name *lname,
                               const struct md_attr *ma);

        int (*mdo_rename_tgt)(const struct lu_env *env, struct md_object *pobj,
                              struct md_object *tobj, const struct lu_fid *fid,
                              const struct lu_name *lname, struct md_attr *ma);
};

struct md_device_operations {
        /* meta-data device related handlers. */
        int (*mdo_root_get)(const struct lu_env *env, struct md_device *m,
                            struct lu_fid *f);

        int (*mdo_maxsize_get)(const struct lu_env *env, struct md_device *m,
                               int *md_size, int *cookie_size);

        int (*mdo_statfs)(const struct lu_env *env, struct md_device *m,
                          struct kstatfs *sfs);

        int (*mdo_init_capa_ctxt)(const struct lu_env *env, struct md_device *m,
                                  int mode, unsigned long timeout, __u32 alg,
                                  struct lustre_capa_key *keys);

        int (*mdo_update_capa_key)(const struct lu_env *env,
                                   struct md_device *m,
                                   struct lustre_capa_key *key);
};

enum md_upcall_event {
        /*sync the md layer*/
        MD_LOV_SYNC = (1 << 0),
        MD_NO_TRANS = (1 << 1), /* Just for split, no need trans, for replay */
        MD_LOV_CONFIG = (1 << 2)
};

struct md_upcall {
        /* this lock protects upcall using against its removal
         * read lock is for usage the upcall, write - for init/fini */
        struct rw_semaphore     mu_upcall_sem;
        /* device to call, upper layer normally */
        struct md_device       *mu_upcall_dev;
        /* upcall function */
        int (*mu_upcall)(const struct lu_env *env, struct md_device *md,
                         enum md_upcall_event ev);
};

struct md_device {
        struct lu_device             md_lu_dev;
        struct md_device_operations *md_ops;
        struct md_upcall             md_upcall;
};

static inline void md_upcall_init(struct md_device *m, void *upcl)
{
        init_rwsem(&m->md_upcall.mu_upcall_sem);
        m->md_upcall.mu_upcall_dev = NULL;
        m->md_upcall.mu_upcall = upcl;
}

static inline void md_upcall_dev_set(struct md_device *m, struct md_device *up)
{
        down_write(&m->md_upcall.mu_upcall_sem);
        m->md_upcall.mu_upcall_dev = up;
        up_write(&m->md_upcall.mu_upcall_sem);
}

static inline void md_upcall_fini(struct md_device *m)
{
        down_write(&m->md_upcall.mu_upcall_sem);
        m->md_upcall.mu_upcall_dev = NULL;
        m->md_upcall.mu_upcall = NULL;
        up_write(&m->md_upcall.mu_upcall_sem);
}

static inline int md_do_upcall(const struct lu_env *env, struct md_device *m,
                               enum md_upcall_event ev)
{
        int rc = 0;
        down_read(&m->md_upcall.mu_upcall_sem);
        if (m->md_upcall.mu_upcall_dev != NULL &&
            m->md_upcall.mu_upcall_dev->md_upcall.mu_upcall != NULL) {
                rc = m->md_upcall.mu_upcall_dev->md_upcall.mu_upcall(env,
                                              m->md_upcall.mu_upcall_dev, ev);
        }
        up_read(&m->md_upcall.mu_upcall_sem);
        return rc;
}

struct md_object {
        struct lu_object             mo_lu;
        struct md_object_operations *mo_ops;
        struct md_dir_operations    *mo_dir_ops;
};

static inline int lu_device_is_md(const struct lu_device *d)
{
        return ergo(d != NULL, d->ld_type->ldt_tags & LU_DEVICE_MD);
}

static inline struct md_device *lu2md_dev(const struct lu_device *d)
{
        LASSERT(lu_device_is_md(d));
        return container_of0(d, struct md_device, md_lu_dev);
}

static inline struct lu_device *md2lu_dev(struct md_device *d)
{
        return &d->md_lu_dev;
}

static inline struct md_object *lu2md(const struct lu_object *o)
{
        LASSERT(lu_device_is_md(o->lo_dev));
        return container_of0(o, struct md_object, mo_lu);
}

static inline struct md_object *md_object_next(const struct md_object *obj)
{
        return (obj ? lu2md(lu_object_next(&obj->mo_lu)) : NULL);
}

static inline struct md_device *md_obj2dev(const struct md_object *o)
{
        LASSERT(lu_device_is_md(o->mo_lu.lo_dev));
        return container_of0(o->mo_lu.lo_dev, struct md_device, md_lu_dev);
}

static inline int md_device_init(struct md_device *md, struct lu_device_type *t)
{
	return lu_device_init(&md->md_lu_dev, t);
}

static inline void md_device_fini(struct md_device *md)
{
	lu_device_fini(&md->md_lu_dev);
}

/* md operations */
static inline int mo_permission(const struct lu_env *env,
                                struct md_object *p,
                                struct md_object *c,
                                struct md_attr *at,
                                int mask)
{
        LASSERT(c->mo_ops->moo_permission);
        return c->mo_ops->moo_permission(env, p, c, at, mask);
}

static inline int mo_attr_get(const struct lu_env *env,
                              struct md_object *m,
                              struct md_attr *at)
{
        LASSERT(m->mo_ops->moo_attr_get);
        return m->mo_ops->moo_attr_get(env, m, at);
}

static inline int mo_readlink(const struct lu_env *env,
                              struct md_object *m,
                              struct lu_buf *buf)
{
        LASSERT(m->mo_ops->moo_readlink);
        return m->mo_ops->moo_readlink(env, m, buf);
}

static inline int mo_attr_set(const struct lu_env *env,
                              struct md_object *m,
                              const struct md_attr *at)
{
        LASSERT(m->mo_ops->moo_attr_set);
        return m->mo_ops->moo_attr_set(env, m, at);
}

static inline int mo_xattr_get(const struct lu_env *env,
                               struct md_object *m,
                               struct lu_buf *buf,
                               const char *name)
{
        LASSERT(m->mo_ops->moo_xattr_get);
        return m->mo_ops->moo_xattr_get(env, m, buf, name);
}

static inline int mo_xattr_del(const struct lu_env *env,
                               struct md_object *m,
                               const char *name)
{
        LASSERT(m->mo_ops->moo_xattr_del);
        return m->mo_ops->moo_xattr_del(env, m, name);
}

static inline int mo_xattr_set(const struct lu_env *env,
                               struct md_object *m,
                               const struct lu_buf *buf,
                               const char *name,
                               int flags)
{
        LASSERT(m->mo_ops->moo_xattr_set);
        return m->mo_ops->moo_xattr_set(env, m, buf, name, flags);
}

static inline int mo_xattr_list(const struct lu_env *env,
                                struct md_object *m,
                                struct lu_buf *buf)
{
        LASSERT(m->mo_ops->moo_xattr_list);
        return m->mo_ops->moo_xattr_list(env, m, buf);
}

static inline int mo_open(const struct lu_env *env,
                          struct md_object *m,
                          int flags)
{
        LASSERT(m->mo_ops->moo_open);
        return m->mo_ops->moo_open(env, m, flags);
}

static inline int mo_close(const struct lu_env *env,
                           struct md_object *m,
                           struct md_attr *ma)
{
        LASSERT(m->mo_ops->moo_close);
        return m->mo_ops->moo_close(env, m, ma);
}

static inline int mo_readpage(const struct lu_env *env,
                              struct md_object *m,
                              const struct lu_rdpg *rdpg)
{
        LASSERT(m->mo_ops->moo_readpage);
        return m->mo_ops->moo_readpage(env, m, rdpg);
}

static inline int mo_object_create(const struct lu_env *env,
                                   struct md_object *m,
                                   const struct md_op_spec *spc,
                                   struct md_attr *at)
{
        LASSERT(m->mo_ops->moo_object_create);
        return m->mo_ops->moo_object_create(env, m, spc, at);
}

static inline int mo_ref_add(const struct lu_env *env,
                             struct md_object *m,
                             const struct md_attr *ma)
{
        LASSERT(m->mo_ops->moo_ref_add);
        return m->mo_ops->moo_ref_add(env, m, ma);
}

static inline int mo_ref_del(const struct lu_env *env,
                             struct md_object *m,
                             struct md_attr *ma)
{
        LASSERT(m->mo_ops->moo_ref_del);
        return m->mo_ops->moo_ref_del(env, m, ma);
}

static inline int mo_capa_get(const struct lu_env *env,
                              struct md_object *m,
                              struct lustre_capa *c,
                              int renewal)
{
        LASSERT(m->mo_ops->moo_capa_get);
        return m->mo_ops->moo_capa_get(env, m, c, renewal);
}

static inline int mo_object_sync(const struct lu_env *env, struct md_object *m)
{
        LASSERT(m->mo_ops->moo_object_sync);
        return m->mo_ops->moo_object_sync(env, m);
}

static inline int mdo_lookup(const struct lu_env *env,
                             struct md_object *p,
                             const struct lu_name *lname,
                             struct lu_fid *f,
                             struct md_op_spec *spec)
{
        LASSERT(p->mo_dir_ops->mdo_lookup);
        return p->mo_dir_ops->mdo_lookup(env, p, lname, f, spec);
}

static inline mdl_mode_t mdo_lock_mode(const struct lu_env *env,
                                       struct md_object *mo,
                                       mdl_mode_t lm)
{
        if (mo->mo_dir_ops->mdo_lock_mode == NULL)
                return MDL_MINMODE;
        return mo->mo_dir_ops->mdo_lock_mode(env, mo, lm);
}

static inline int mdo_create(const struct lu_env *env,
                             struct md_object *p,
                             const struct lu_name *lchild_name,
                             struct md_object *c,
                             struct md_op_spec *spc,
                             struct md_attr *at)
{
        LASSERT(c->mo_dir_ops->mdo_create);
        return c->mo_dir_ops->mdo_create(env, p, lchild_name, c, spc, at);
}

static inline int mdo_create_data(const struct lu_env *env,
                                  struct md_object *p,
                                  struct md_object *c,
                                  const struct md_op_spec *spec,
                                  struct md_attr *ma)
{
        LASSERT(c->mo_dir_ops->mdo_create_data);
        return c->mo_dir_ops->mdo_create_data(env, p, c, spec, ma);
}

static inline int mdo_rename(const struct lu_env *env,
                             struct md_object *sp,
                             struct md_object *tp,
                             const struct lu_fid *lf,
                             const struct lu_name *lsname,
                             struct md_object *t,
                             const struct lu_name *ltname,
                             struct md_attr *ma)
{
        LASSERT(tp->mo_dir_ops->mdo_rename);
        return tp->mo_dir_ops->mdo_rename(env, sp, tp, lf, lsname, t, ltname,
                                          ma);
}

static inline int mdo_is_subdir(const struct lu_env *env,
                                struct md_object *mo,
                                const struct lu_fid *fid,
                                struct lu_fid *sfid)
{
        LASSERT(mo->mo_dir_ops->mdo_is_subdir);
        return mo->mo_dir_ops->mdo_is_subdir(env, mo, fid, sfid);
}

static inline int mdo_link(const struct lu_env *env,
                           struct md_object *p,
                           struct md_object *s,
                           const struct lu_name *lname,
                           struct md_attr *ma)
{
        LASSERT(s->mo_dir_ops->mdo_link);
        return s->mo_dir_ops->mdo_link(env, p, s, lname, ma);
}

static inline int mdo_unlink(const struct lu_env *env,
                             struct md_object *p,
                             struct md_object *c,
                             const struct lu_name *lname,
                             struct md_attr *ma)
{
        LASSERT(c->mo_dir_ops->mdo_unlink);
        return c->mo_dir_ops->mdo_unlink(env, p, c, lname, ma);
}

static inline int mdo_name_insert(const struct lu_env *env,
                                  struct md_object *p,
                                  const struct lu_name *lname,
                                  const struct lu_fid *f,
                                  const struct md_attr *ma)
{
        LASSERT(p->mo_dir_ops->mdo_name_insert);
        return p->mo_dir_ops->mdo_name_insert(env, p, lname, f, ma);
}

static inline int mdo_name_remove(const struct lu_env *env,
                                  struct md_object *p,
                                  const struct lu_name *lname,
                                  const struct md_attr *ma)
{
        LASSERT(p->mo_dir_ops->mdo_name_remove);
        return p->mo_dir_ops->mdo_name_remove(env, p, lname, ma);
}

static inline int mdo_rename_tgt(const struct lu_env *env,
                                 struct md_object *p,
                                 struct md_object *t,
                                 const struct lu_fid *lf,
                                 const struct lu_name *lname,
                                 struct md_attr *ma)
{
        if (t) {
                LASSERT(t->mo_dir_ops->mdo_rename_tgt);
                return t->mo_dir_ops->mdo_rename_tgt(env, p, t, lf, lname, ma);
        } else {
                LASSERT(p->mo_dir_ops->mdo_rename_tgt);
                return p->mo_dir_ops->mdo_rename_tgt(env, p, t, lf, lname, ma);
        }
}

#endif /* _LINUX_MD_OBJECT_H */
