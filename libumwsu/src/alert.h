#ifndef _LIBUMWSU_ALERT_H_
#define _LIBUMWSU_ALERT_H_

void alert_callback(struct umwsu_report *report, void *callback_data);

extern struct umwsu_module umwsu_mod_alert;

#endif
