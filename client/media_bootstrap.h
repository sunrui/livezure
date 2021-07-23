/*
 * service rpc entry
 *
 * @author rui.sun 2012-7-22 Sun 12:52
 */
#ifndef MEDIA_MODULE_H
#define MEDIA_MODULE_H

/*
 * all client pre-init will be start here 
 */
void media_bootstrap_start();

/*
 * client destroy module when no longer used
 */
void media_bootstrap_clean();

#endif