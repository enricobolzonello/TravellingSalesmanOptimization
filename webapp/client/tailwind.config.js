/** @type {import('tailwindcss').Config} */
export default {
  content: ["./src/**/*.{html,js,jsx}", 
  "./src/components/*.{js,jsx}",
  "./node_modules/tw-elements/js/**/*.js"
],
  theme: {
    extend: {
      borderWidth: {
        DEFAULT: '1px',
        '0': '0',
        '2': '2px',
        '3': '3px',
        '4': '4px',
        '6': '6px',
        '8': '8px',
      }
    },
  },
  plugins: [
    require("tw-elements/plugin.cjs"),
    require('@tailwindcss/forms'),
    require('@tailwindcss/typography'),
],
  darkMode: "class"
}

