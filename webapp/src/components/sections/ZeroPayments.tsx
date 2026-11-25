import Gradient from "../../assets/gradient.jpg"

export default function ZeroPayments() {
    return (<div className="my-30 site-max flex gap-10 md:flex-row-reverse flex-col">

        <div className="flex-1 sticky top-[100px] md:self-start text-right">
            <h1 className="text-4xl">Zero Payment</h1>
            <p className="md:text-md text-muted-foreground mt-4">Lorem ipsum dolor sit amet consectetur adipisicing elit. Impedit quibusdam neque iusto nobis ipsam animi voluptate natus numquam quas, ut optio accusamus aliquid maxime perferendis dicta reprehenderit.</p>
        </div>

        <div className="flex-1 aspect-square">
            <div className="rounded-md h-full w-full relative overflow-hidden">
                <img src={Gradient.src} className="w-full h-full top-0 left-0 absolute object-cover z-[-1]" />
            

            
            </div>
        </div>

    </div>)
}