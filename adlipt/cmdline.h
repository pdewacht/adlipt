enum mode { MODE_LOAD, MODE_UNLOAD, MODE_STATUS, MODE_USAGE };

enum mode parse_command_line(char _WCI86FAR *str);

int *collect_ports(struct config _WCI86FAR *cfg);
