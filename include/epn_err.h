#if !defined __EPN_ERR_H_
#define __EPN_ERR_H_

#ifdef __cplusplus
extern "C" {
#endif

/*! \file epn_err.h
 *  \brief epn error codes.
 */

const int EPN_NOPIPE    = -1000;
const int EPN_NOSOCK    = -1001;
const int EPN_NORUADDR  = -1002;
const int EPN_NOBIND    = -1003;
const int EPN_NOSETBLK  = -1004;
const int EPN_NOLISTEN  = -1005;
const int EPN_NOTHREAD  = -1006;
const int EPN_NOJOIN    = -1007;
const int EPN_NOEPOLL   = -1008;
const int EPN_NOEPADD   = -1009;

#ifdef __cplusplus
}
#endif

#endif /* __EPN_ERR_H_ */
