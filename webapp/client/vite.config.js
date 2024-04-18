import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react-swc'


// https://vitejs.dev/config/
export default defineConfig({
  plugins: [react()],
  server: {
    proxy: {
      "/run": "http://localhost:8080/",
      "/palle" : "http://localhost:8080/",
    },
  },
})
