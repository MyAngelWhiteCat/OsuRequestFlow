export const API_BASE_URL = 'http://localhost:8181/api'

export const ENDPOINTS = {
  SETTINGS: {
    LOAD: '/load_settings',
    SAVE: '/save_settings',
    MAX_FILE_SIZE: '/set_max_filesize',
    DOWNLOADS_FOLDER: '/set_downloads_folder',
    DOWNLOADS_RESOURCE: '/set_dl_resourse',
    ROLE_FILTER_LEVEL: '/set_role_filter_lvl',
    WHITELIST_ONLY: '/set_whitelist_only',
    RECONNECT_TIMEOUT: '/set_reconnect_timeout',
  },
  LISTS: {
    WHITELIST: '/whitelist',
    BLACKLIST: '/blacklist',
  },
  IRC: {
    JOIN_CHANNEL: '/irc_client/join',
    PART_CHANNEL: '/irc_client/part',
    TIMEOUT: '/irc_client/settings/set_reconnect_timeout'
  },
}