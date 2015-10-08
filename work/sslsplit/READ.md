* log function *
logbuf_write_free: log thread call this to write data and free, and finally call log_content_common_writecb
log_content_submit: submit content to log thread
log_content_open: open one log file(ssn)    -- pxy_bev_eventcb
log_content_close: close one log file(ssn)  -- pxy_conn_ctx_free

key func: pxy_bev_readcb:1791
