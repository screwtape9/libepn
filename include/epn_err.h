#if !defined __EPN_ERR_H_
#define __EPN_ERR_H_

#ifdef __cplusplus
extern "C" {
#endif

/*! \file epn_err.h
 *  \brief epn error codes.
 */

extern const int EPN_NOSOCK;
extern const int EPN_NORUADDR;
extern const int EPN_NOBIND;
extern const int EPN_NOSETBLK;
extern const int EPN_NOLISTEN;
extern const int EPN_NOTHREAD;
extern const int EPN_NOJOIN;
extern const int EPN_NOEPOLL;
extern const int EPN_NOEPADD;

#ifdef __cplusplus
}
#endif

#endif /* __EPN_ERR_H_ */
