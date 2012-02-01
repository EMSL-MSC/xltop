#include <malloc.h>
#include "cl_conn.h"
#include "auth.h"
#include "clus.h"
#include "serv.h"
#include "user.h"
#include "string1.h"
#include "trace.h"

int
clus_connect(EV_P_ struct cl_conn *cc, char *ctl, char *args, size_t args_len)
{
  char *name, *user, *stime, *sig;
  struct x_node *x;
  struct clus_node *c;

  if (split(&args, &name, &user, &stime, &sig, (char *) NULL) != 4)
    return CL_ERR_NR_ARGS;

  x = x_lookup(X_CLUS, name, 0);
  if (x == NULL)
    return CL_ERR_NO_CLUS;

  c = container_of(x, struct clus_node, c_x);
  if (!auth_ctl_is_allowed(c->c_auth, ev_now(EV_A), ctl, name, user, stime, sig))
    return CL_ERR_NO_AUTH;

  if (cl_conn_transfer(EV_A_ &c->c_conn, cc) < 0)
    return CL_ERR_INTERNAL;

  TRACE("clus `%s' connected\n", name);

  return CL_CONN_TRANSFERRED;
}

int
serv_connect(EV_P_ struct cl_conn *cc, char *ctl, char *args, size_t args_len)
{
  char *name, *user, *stime, *sig;
  struct x_node *x;
  struct serv_node *s;

  if (split(&args, &name, &user, &stime, &sig, (char *) NULL) != 4)
    return CL_ERR_NR_ARGS;

  x = x_lookup(X_SERV, name, 0);
  if (x == NULL)
    return CL_ERR_NO_SERV;
  s = container_of(x, struct serv_node, s_x);

  if (!auth_ctl_is_allowed(s->s_auth, ev_now(EV_A), ctl, name, user, stime, sig))
    return CL_ERR_NO_AUTH;

  if (cl_conn_transfer(EV_A_ &s->s_conn, cc) < 0)
    return CL_ERR_INTERNAL;

  TRACE("serv `%s' connected\n", name);

  return CL_CONN_TRANSFERRED;
}

int
user_connect(EV_P_ struct cl_conn *cc, char *ctl, char *args, size_t args_len)
{
  char *name, *user, *stime, *sig;
  struct user_domain *ud;
  struct user_conn *uc = NULL;
  int err;

  if (split(&args, &name, &user, &stime, &sig, (char *) NULL) != 4)
    return CL_ERR_NR_ARGS;

  /* FIXME */
  ud = user_domain_lookup(user, 0);
  if (ud == NULL) {
    err = CL_ERR_NO_USER;
    goto out;
  }

  if (!auth_ctl_is_allowed(ud->d_auth, ev_now(EV_A), ctl, name, user, stime, sig)) {
    err = CL_ERR_NO_AUTH;
    goto out;
  }

  uc = malloc(sizeof(*uc));
  if (uc == NULL) {
    err = CL_ERR_NO_MEM;
    goto out;
  }

  if (user_conn_init(uc, name, user) < 0) {
    err = CL_ERR_NO_MEM;
    goto out;
  }

  if (cl_conn_transfer(EV_A_ &uc->c_conn, cc) < 0) {
    err = CL_ERR_INTERNAL;
    goto out;
  }

  TRACE("user `%s' `%s' connected\n", name, user);

  return CL_CONN_TRANSFERRED;

 out:
  if (uc != NULL) {
    user_conn_stop(EV_A_ uc);
    user_conn_destroy(uc);
  }
  free(uc);

  return err;
}

static struct cl_conn_ctl new_conn_ctl_table[] = {
  { .cc_ctl_cb = &clus_connect, .cc_ctl_name = "clus_connect" },
  { .cc_ctl_cb = &serv_connect, .cc_ctl_name = "serv_connect" },
  { .cc_ctl_cb = &user_connect, .cc_ctl_name = "user_connect" },
};

struct cl_conn_ops new_conn_ops = {
  .cc_ctl = new_conn_ctl_table,
  .cc_nr_ctl = sizeof(new_conn_ctl_table) / sizeof(new_conn_ctl_table[0]),
  .cc_timeout = 60,
  .cc_rd_buf_size = 4096,
};
