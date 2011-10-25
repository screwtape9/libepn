#if !defined __EPN_ERR_H_
#define __EPN_ERR_H_

#ifdef __cplusplus
extern "C" {
#endif

/*! \file epn_err.h
 *  \brief epn error codes.
 */

static const int EPN_NOSOCK    = -1000;
static const int EPN_NORUADDR  = -1001;
static const int EPN_NOBIND    = -1002;
static const int EPN_NOSETBLK  = -1003;
static const int EPN_NOLISTEN  = -1004;
static const int EPN_NOTHREAD  = -1005;
static const int EPN_NOJOIN    = -1006;
static const int EPN_NOEPOLL   = -1007;
static const int EPN_NOEPADD   = -1008;

#ifdef __cplusplus
}
#endif

#endif /* __EPN_ERR_H_ */
