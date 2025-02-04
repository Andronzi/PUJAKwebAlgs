import { config } from "dotenv"; config()
import path from "path"
import url from "url"
import express from "express";
// import handlebars from "express-handlebars"
import pagesRouter from "./src/routes/pages.js"
import algsRouter from "./src/routes/algs.js"
import { binStreamParser } from "./src/middlewares.js"
import algorithms from "./src/algorithms.cjs"
const __dirname = path.dirname(url.fileURLToPath(import.meta.url))

if (!process.env.weightsName || !process.env.biasesName) {
    throw ".env file must weightsName and biasesName";
}

algorithms.neuralNet.init(
    path.join(__dirname, "resources", process.env.weightsName),
    path.join(__dirname, "resources", process.env.biasesName)
)

log('log', process.env.weightsName, process.env.biasesName);

if (!process.env["jwtSecret"]) {
    throw ".env file must contain 'jwtSecret'";
}

const app = express()
// const hbs = handlebars.create({
//     defaultLayout: "main",
//     extname: "hbs"
// })

// app.engine("hbs", hbs.engine)
//     .set("view engine", "hbs")
app.set("views", path.join(__dirname, "views"))
    .set("rootPath", __dirname)

app.use(express.json({limit: '50mb'}))
    .use(express.static(path.join(__dirname, "public")))
    .use(binStreamParser)
    .use(pagesRouter)
    .use("/alg", algsRouter)


const port = process.env.CONNECTION_PORT
if (!port) throw "Enviroment variable 'CONNECTION_PORT' isn't initialized"
try {

    app.listen(port, () => {
        console.log(`Server has been started on port ${port}...`)
    })
} 
catch(e) {
    console.log(e)
}