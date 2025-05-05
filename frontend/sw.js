self.addEventListener('install', () => {
    console.log("✅ Service worker installed");
  });
  
  self.addEventListener('fetch', (event) => {
    event.respondWith(fetch(event.request));
  });
  