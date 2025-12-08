import axios from 'axios'
import { API_BASE_URL } from './endpoints'

const client = axios.create({
  baseURL: API_BASE_URL,
  timeout: 10000,
  headers: {
    'Content-Type': 'application/json',
  },
})

// client.interceptors.response.use(
  
// )

export default client