import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react-swc'


// https://vitejs.dev/config/
export default defineConfig({
  base: "/",
  plugins: [react()],
  assetsInclude: ['**/*.md'],
  define: {
    global: {},
  },
  server: {
    port: 3000,
    strictPort: true,
    host: true,
    origin: "http://localhost:3000",
    /*proxy: {
      "/run": "http://localhost:8080/",
      "/palle" : "http://localhost:8080/",
    },*/
  },
  preview: {
    port: 3000,
    strictPort: true,
   }
})
