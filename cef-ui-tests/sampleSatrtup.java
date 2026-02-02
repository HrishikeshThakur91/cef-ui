public class YourMainApp {
    private CefApplicationManager cefManager;
    
    // At startup
    public void onStartup() {
        cefManager = new CefApplicationManager();
        cefManager.initializeServers(
            "C:/docs/.vuepress/dist", 8080, 50051
        );
    }
    
    // On button click
    public void onOpenBrowserClick() {
        cefManager.launchCefBrowser(
            cefManager.getHttpServerUrl()
        );
    }
    
    // On shutdown
    public void onShutdown() {
        cefManager.shutdown();
    }
}