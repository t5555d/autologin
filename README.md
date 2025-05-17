# Autologin credential provider for Microsoft Windows

Credential provider for windows login screen:
- Uses DefaultUserName and DefaultPassword from registry to login, if AutoAdminLogon is set;
- Logins automatically, if no RDP connection is set;
- Logins automatically after RDP connection is closed.
