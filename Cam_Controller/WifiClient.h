class WifiClient
{
    public:
        WifiClient(char *ssid, char *pass);
        void connect();
        void disconnect();

    private:
        char *_ssid;
        char *_pass;
};